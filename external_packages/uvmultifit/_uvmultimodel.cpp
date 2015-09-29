#include <Python.h>
#include <numpy/arrayobject.h>
#include <pthread.h>
#include <gsl/gsl_sf_bessel.h>
#include <stdio.h>  
#include <sys/types.h>
#include <new>

/* Docstrings */
static char module_docstring[] =
    "This module provides an interface for least-square visibility fitting.";
static char uvmultimodel_docstring[] =
    "Calculate the residuals and chi square of a multi-component model";
static char setData_docstring[] =
    "Get the data pointers.";
static char setNspw_docstring[] =
    "Set up the pointers to the data arrays.";
static char setModel_docstring[] =
    "Set up the model components and variable parameters.";
static char setNCPU_docstring[] =
    "Set up the parallelization.";
static char setWork_docstring[] =
    "Allocate memory to compute Hessian and error vector.";
static char unsetWork_docstring[] =
    "Deallocate memory obtained with setWork().";


/* Available functions */
static PyObject *setNspw(PyObject *self, PyObject *args);
static PyObject *setData(PyObject *self, PyObject *args);
static PyObject *setNCPU(PyObject *self, PyObject *args);
static PyObject *modelcomp(PyObject *self, PyObject *args);
static PyObject *setModel(PyObject *self, PyObject *args);
static PyObject *setWork(PyObject *self, PyObject *args);
static PyObject *unsetWork(PyObject *self, PyObject *args);


void *writemod(void *work);


/* Module specification */
static PyMethodDef module_methods[] = {
    {"setData", setData, METH_VARARGS, setData_docstring},
    {"setNspw", setNspw, METH_VARARGS, setNspw_docstring},
    {"setModel", setModel, METH_VARARGS, setModel_docstring},
    {"setNCPU", setNCPU, METH_VARARGS, setNCPU_docstring},
    {"modelcomp", modelcomp, METH_VARARGS, uvmultimodel_docstring},
    {"setWork", setWork, METH_VARARGS, setWork_docstring},
    {"unsetWork", unsetWork, METH_VARARGS, unsetWork_docstring},
    {NULL, NULL, 0, NULL}
};

double **vars, **fixp, **freqs;
double **uv[2], **ObsVis[2], **wgt[2], **ModVis[2], **dt;
double *Chi2;
double **WorkHess, **WorkGrad; 
char **fittable;

/* Structure to pass, as void cast, to the workers */
struct SHARED_DATA {
int *t0;
int *t1;
int Iam;
};

static SHARED_DATA master;
static SHARED_DATA *worker;
static int *nnu, *nt, *models, mode;
static int Nspw, NCPU, nui, cIF, ncomp = 1, npar=1, maxnchan=0;
double *Hessian, *Gradient, *dpar, *muRA, *muDec;


/* Initialize the module */
PyMODINIT_FUNC init_uvmultimodel(void)
{
    PyObject *m = Py_InitModule3("_uvmultimodel", module_methods, module_docstring);
    if (m == NULL)
        return;



//////////////////////
// Initiate variables with dummy values:
    NCPU = 1; Nspw = 1;
    worker = new SHARED_DATA[1];
    master.t0 = new int[1];
    master.t1 = new int[1];
    worker[0].t0 = new int[1];
    worker[0].t1 = new int[1];
    nnu = new int[1];
    nt = new int[1];

    Chi2 = new double[1];
    vars = new double*[2];
    fixp = new double*[1];

    WorkHess = new double*[1];
    WorkGrad = new double*[1];
    WorkHess[0] = new double[1];
    WorkGrad[0] = new double[1];

    Hessian = new double[1];
    models = new int[1];
    Gradient = new double[1];
    dpar = new double[1];;
    muRA = new double[1];;
    muDec = new double[1];;

////////////////////



    /* Load `numpy` functionality. */
    import_array();
}




///////////////////////////////////////
/* Code for the workers (NOT CALLABLE FROM PYTHON). */

void *writemod(void *work) {

SHARED_DATA *mydata = (SHARED_DATA *)work;


int nu0, nu1  ;
if (nui==-1){nu0=0;nu1=nnu[cIF];}else{nu0=nui;nu1=nui+1;};

int k,i,t,j,m,p, currow[6];

double phase, uu, vv, UVRad, Ampli, DerivRe[npar], DerivIm[npar];
double SPA, CPA, tA, tB, tempChi, deltat;
double wgtcorrA, tempRe[npar+1], tempIm[npar+1];
tempChi = 0.0;

const double deg2rad = 0.017453292519943295;

int Iam = mydata->Iam; 
int widx = (Iam == -1)?0:Iam;
int pmax = (mode == -1)?npar+1:1;
int mmax = (mode == 0)?1:ncomp;
bool write2mod = (mode == -3 || mode >= 0);
bool allmod = (mode == -3);
bool writeDer = (mode==-1);

double tempD, cosphase, sinphase, PA;
//double auxPhase, auxUVRad, auxPA;




for (t = mydata->t0[cIF]; t < mydata->t1[cIF] ; t++) {


 deltat = dt[cIF][t];

 if (fittable[cIF][t]!=0) {

  for (i = nu0; i < nu1; i++) {

    j = (nui!=-1)?0:i;
    k = nnu[cIF]*t+i;
    tempD = pow(wgt[0][cIF][k],2.);
 //   tempD = wgt[0][cIF][k];

    uu = freqs[cIF][i]*uv[0][cIF][t];
    vv =  freqs[cIF][i]*uv[1][cIF][t];

    for(p=0;p<pmax;p++){
      tempRe[p]=0.0; tempIm[p]=0.0;
    };

    for (m=0;m<mmax;m++){

   /*   auxUVRad = -1.0;
      auxPhase = 0.0;
      cosphase = 1.0;
      sinphase = 0.0;
      phase = 0.0;
      UVRad = 0.0;
      Ampli = 1.0;
      auxPA = 0.0;
      PA= 0.0; 
      SPA = 0.0; CPA = 1.0;  */

      for(p=0;p<6;p++){currow[p] = m*maxnchan*6+j+p*maxnchan;};
        wgtcorrA = exp(wgt[1][cIF][m*nt[cIF]+t]*pow(freqs[cIF][i],2.));

      for(p=0;p<pmax;p++){

        phase = (vars[p][currow[0]]+muRA[m]*deltat)*uu + (vars[p][currow[1]]+muDec[m]*deltat)*vv;
    //    if (phase != auxPhase){
    //      auxPhase = phase;
          cosphase = cos(phase);
          sinphase = sin(phase);
    //    };


        if (models[m] != 0) {
         PA = vars[p][currow[5]]*deg2rad;
    //     if (auxPA != PA){
           SPA = sin(PA);
           CPA = cos(PA);
     //      auxPA = PA;
     //    };
         tA = (uu*CPA - vv*SPA)*vars[p][currow[4]];
         tB = (uu*SPA + vv*CPA);
         UVRad = pow(tA*tA+tB*tB,0.5)*(vars[p][currow[3]]/2.0);



    //     if (UVRad != auxUVRad) {
    //      auxUVRad = UVRad;

         if (UVRad > 0.0) {
          switch (models[m]) {
            case 1: Ampli = (vars[p][currow[2]])*exp(-0.3606737602*UVRad*UVRad); break;
            case 2: Ampli = 2.0*(vars[p][currow[2]])*gsl_sf_bessel_J1(UVRad)/UVRad; break;
            case 3: Ampli = (vars[p][currow[2]])*gsl_sf_bessel_J0(UVRad); break;
            case 4: Ampli = 3.0*(vars[p][currow[2]])*(sin(UVRad)-UVRad*cos(UVRad))*(pow(UVRad,-3.0)); break;
            case 5: Ampli = (vars[p][currow[2]])*sin(UVRad)/UVRad; break;
            case 6: Ampli = (vars[p][currow[2]])*pow(1.+0.52034224525*UVRad*UVRad,-1.5); break;
            case 7: Ampli = 0.459224094*(vars[p][currow[2]])*gsl_sf_bessel_K0(UVRad); break;
            case 8: Ampli = (vars[p][currow[2]])*exp(-UVRad*1.3047660265); break;
            default: Ampli = vars[p][currow[2]];
          };
        } else {wgt[0][cIF][k]=0.0; Ampli=vars[p][currow[2]];};


    //    };

       } else {Ampli = vars[p][currow[2]];};

    Ampli *= wgtcorrA;

    tempRe[p] += Ampli*cosphase;
    tempIm[p] += Ampli*sinphase;

    if(allmod && m==0){
       ModVis[0][cIF][k] *= fixp[p][j];
       ModVis[1][cIF][k] *= fixp[p][j];
    };

    if(allmod || m==0) {
      if(write2mod){
         ModVis[0][cIF][k] += tempRe[p];
         ModVis[1][cIF][k] += tempIm[p];
      } else if(m==0) {
         tempRe[p] += ModVis[0][cIF][k]*fixp[p][j];
         tempIm[p] += ModVis[1][cIF][k]*fixp[p][j];
      };
    };

  };

  };

  tempChi += pow((ObsVis[0][cIF][k]-tempRe[0])*wgt[0][cIF][k],2.);
  tempChi += pow((ObsVis[1][cIF][k]-tempIm[0])*wgt[0][cIF][k],2.);


   if (writeDer) {

    for(p=0;p<npar;p++){
      DerivRe[p] = (tempRe[p+1]-tempRe[0])/dpar[p]; 
      DerivIm[p] = (tempIm[p+1]-tempIm[0])/dpar[p]; 

      WorkGrad[widx][p] += (ObsVis[0][cIF][k]-tempRe[0])*tempD*DerivRe[p];
      WorkGrad[widx][p] += (ObsVis[1][cIF][k]-tempIm[0])*tempD*DerivIm[p];
    };  

    for(p=0;p<npar;p++){
      for(m=p;m<npar;m++){
        WorkHess[widx][npar*p+m] += tempD*(DerivRe[p]*DerivRe[m] + DerivIm[p]*DerivIm[m]);
      };
    };

   };



  };

  };


};

if (writeDer){
for(p=0;p<npar;p++){
  for(m=p;m<npar;m++){
    WorkHess[widx][npar*m+p] = WorkHess[widx][npar*p+m];
  };
};
};

if (Iam != -1){Chi2[Iam] = tempChi; pthread_exit((void*) 0);} 
else {Chi2[0] = tempChi; return (void*) 0;};

}
// END OF CODE FOR THE WORKERS
//////////////////////////////////////




//////////////////////////////////////
// SET THE NUMBER OF IFS (IT REINITIATES ALL SHARED DATA VARIABLES!):
// USAGE FROM PYTHON: setNspw(i) where i is the number of SPW
static PyObject *setNspw(PyObject *self, PyObject *args)
{
    int i;
    if (!PyArg_ParseTuple(args, "i",&i)){printf("FAILED setNspw!\n"); fflush(stdout); return NULL;}

    delete[] nnu; 
    delete[] nt; 
    delete[] master.t0; 
    delete[] master.t1;
    
    nnu = new int[i];
    nt = new int[i];
    master.t0 = new int[i];
    master.t1 = new int[i];

    freqs = new double*[i];
    uv[0] = new double*[i];
    uv[1] = new double*[i];
    ObsVis[0] = new double*[i];
    ObsVis[1] = new double*[i];
    ModVis[0] = new double*[i];
    ModVis[1] = new double*[i];
    fittable = new char*[i];
    wgt[1] = new double*[i];
    wgt[0] = new double*[i];
    dt = new double*[i];
    Nspw = i;



    PyObject *ret = Py_BuildValue("i",0);
    return ret;
}


/////////////////////////////
// PREPARE PARALLELIZATION (MUST BE RUN AFTER setData)
// USAGE FROM PYTHON: setNCPU(i) where i is the num. of threads allowed
static PyObject *setNCPU(PyObject *self, PyObject *args)
{
    int i, j, k, k0, k1;
    if (!PyArg_ParseTuple(args, "i",&i)){printf("FAILED setNCPU!\n"); fflush(stdout);  return NULL;};

    printf("Preparing memory for %i workers\n",i);

    for(j=0; j<NCPU; j++){
      delete[] worker[j].t0; 
      delete[] worker[j].t1;
    };

    delete[] worker;

    worker = new SHARED_DATA[i];
    for(k=0; k<i; k++){
      worker[k].t0 = new int[Nspw];
      worker[k].t1 = new int[Nspw];
      worker[k].Iam = k;
    };


    for (j=0; j<Nspw; j++) {
         int nperproc = (int)((double)nt[j])/((double)i);
         for (k=0; k<i; k++){
            k0 = nperproc*k;
            if (k==i-1){k1=nt[j];}else{k1 = nperproc*(k+1);};
            worker[k].t0[j] = k0;
            worker[k].t1[j] = k1;
         };
    };


    delete[] Chi2;


    delete[] WorkHess;
    delete[] WorkGrad;
    WorkHess = new double*[i];
    WorkGrad = new double*[i];
    Chi2 = new double[i];

    NCPU = i;


    PyObject *ret = Py_BuildValue("i",0);
    return ret;
}




//////////////////////////////////
// Fill-in the DATA arrays and master SHARED_DATA object
// USAGE FROM PYTHON: setData(IF, arrlist) where arrlist is list of data arrays.
//                    and IF is the IF number (setNspw must be run first!)
static PyObject *setData(PyObject *self, PyObject *args)
{

    PyObject *pu, *pv, *pwgt, *preal, *pimag, *poreal, *poimag;
    PyObject *pfreqs, *pfittable;
    PyObject *pwgtcorr, *dtime;
    int IF;

    if (!PyArg_ParseTuple(args, "iOOOOOOOOOOO",&IF,&pu,&pv,&pwgt,&preal,&pimag,&poreal,&poimag,&pfreqs,&pfittable,&pwgtcorr, &dtime)){printf("FAILED setData!\n"); fflush(stdout);  return NULL;};




    /* Interprete the input objects as numpy arrays. */

    uv[0][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(pu, NPY_DOUBLE, NPY_IN_ARRAY));
    uv[1][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(pv, NPY_DOUBLE, NPY_IN_ARRAY));
    wgt[0][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(pwgt, NPY_DOUBLE, NPY_IN_ARRAY));

    ObsVis[0][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(preal, NPY_DOUBLE, NPY_IN_ARRAY));
    ObsVis[1][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(pimag, NPY_DOUBLE, NPY_IN_ARRAY));


    ModVis[0][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(poreal, NPY_DOUBLE, NPY_IN_ARRAY));
    ModVis[1][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(poimag, NPY_DOUBLE, NPY_IN_ARRAY));

    freqs[IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(pfreqs, NPY_DOUBLE, NPY_IN_ARRAY));

    fittable[IF] = (char *)PyArray_DATA(PyArray_FROM_OTF(pfittable, NPY_INT8, NPY_IN_ARRAY));
    wgt[1][IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(pwgtcorr, NPY_DOUBLE, NPY_IN_ARRAY));

    dt[IF] = (double *)PyArray_DATA(PyArray_FROM_OTF(dtime, NPY_DOUBLE, NPY_IN_ARRAY));

    nt[IF] = PyArray_DIM(preal,0);
    nnu[IF] = PyArray_DIM(preal,1);




    if(nnu[IF]>maxnchan){maxnchan=nnu[IF];};


    PyObject *ret = Py_BuildValue("i",10);
    return ret;

}



//////////////////////////////////
// Fill-in the MODEL arrays
// USAGE FROM PYTHON: setModel(arrlist) where arrlist is list of data arrays.
PyObject *setModel(PyObject *self, PyObject *args) {

    PyObject *HessArr, *GradArr, *modArr, *VarArr, *FixArr, *tempArr, *dparArr, *propRA, *propDec;
    int i;

    if (!PyArg_ParseTuple(args, "OOOOOOOO",&modArr,&HessArr,&GradArr,&VarArr,&FixArr,&dparArr, &propRA, &propDec)){printf("FAILED setModel!\n"); fflush(stdout);  return NULL;};

//printf("STEP 1!\n");

  //  delete[] Hessian;
  //  delete[] models;
  //  delete[] Gradient;
  //  delete[] dpar;
  //  delete[] muRA;
  //  delete[] muDec;

//printf("STEP 2!\n");
    

    models = (int *)PyArray_DATA(PyArray_FROM_OTF(modArr, NPY_INT32, NPY_IN_ARRAY));
    Hessian = (double *)PyArray_DATA(PyArray_FROM_OTF(HessArr, NPY_DOUBLE, NPY_IN_ARRAY));
    Gradient = (double *)PyArray_DATA(PyArray_FROM_OTF(GradArr, NPY_DOUBLE, NPY_IN_ARRAY));
    dpar = (double *)PyArray_DATA(PyArray_FROM_OTF(dparArr, NPY_DOUBLE, NPY_IN_ARRAY));
    muRA = (double *)PyArray_DATA(PyArray_FROM_OTF(propRA, NPY_DOUBLE, NPY_IN_ARRAY));
    muDec = (double *)PyArray_DATA(PyArray_FROM_OTF(propDec, NPY_DOUBLE, NPY_IN_ARRAY));

//printf("STEP 3!\n");


    ncomp = PyArray_DIM(modArr,0);
    npar = PyArray_DIM(GradArr,0);

 //   printf("npar = %i ; ncomp = %i\n",npar,ncomp);fflush(stdout);
//printf("STEP 4!\n");

    delete[] vars;
    delete[] fixp;
  //  delete[] muRA;
  //  delete[] muDec;

//printf("STEP 5!\n");

    vars = new double*[(npar+1)];
    fixp = new double*[(npar+1)];
//    muRA = new double[ncomp];
//    muDec = new double[ncomp];

//printf("STEP 6!\n");


    for (i=0; i<(npar+1); i++) {
      tempArr = PyList_GetItem(VarArr,i);
      vars[i] = (double *)PyArray_DATA(PyArray_FROM_OTF(tempArr, NPY_DOUBLE, NPY_IN_ARRAY));
    };


    for (i=0; i<(npar+1); i++) {
      tempArr = PyList_GetItem(FixArr,i);
      fixp[i] = (double *)PyArray_DATA(PyArray_FROM_OTF(tempArr, NPY_DOUBLE, NPY_IN_ARRAY));
    };

//printf("STEP 7!\n");

    PyObject *ret = Py_BuildValue("i",10);
    return ret;



};

//////////////////////////////////
// Allocate memory for the workers.
// USAGE FROM PYTHON: setWork() with no arguments.
//                    (setNCPU and setModel must be run first!)
static PyObject *setWork(PyObject *self, PyObject *args)
{
  int i;
    for (i=0; i<NCPU; i++) {
      WorkHess[i] = new double[npar*npar];
      WorkGrad[i] = new double[npar];
    };


    PyObject *ret = Py_BuildValue("i",10);
    return ret;

};


/////////////////////////////////
// Deallocate the memory allocated with setWork.
// USAGE FROM PYTHON: unsetWork() with no arguments.
//                    (obviously, setWork must be run first!)
static PyObject *unsetWork(PyObject *self, PyObject *args)
{
  int i;

  for(i=0;i<NCPU;i++){
    delete WorkHess[i];
    delete WorkGrad[i];  
  };

  //  delete WorkHess;
  //  delete WorkGrad;

    PyObject *ret = Py_BuildValue("i",10);
    return ret;

};


//////////////////////////////////////////////////
/* Main Python function. It spreads the work through the workers */
// USAGE FROM PYTHON: modelcomp(IF, nui, opts) (see Python code for info).
static PyObject *modelcomp(PyObject *self, PyObject *args)
{
    void *status;
    double totChi=0.0;
    int i,j;
    int nparsq = npar*npar;


    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "iii",&cIF,&nui,&mode)){printf("FAILED modelcomp!\n"); fflush(stdout);  return NULL;};

//printf("Setting memory %i \n",NCPU);

// Zero the workers memory:
    for (i=0; i<NCPU; i++) {
      for(j=0;j<nparsq;j++){WorkHess[i][j] = 0.0;};
      for(j=0;j<npar;j++){WorkGrad[i][j] = 0.0;};
    };



if (NCPU>1) {

  /* Code for the case NCPU>1. 
     Define the workers and perform the parallel task. */

  pthread_t MyThreads[NCPU];
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


  for (i=0; i<NCPU; i++){
//    printf("Creating thread %i \n",i);
    pthread_create(&MyThreads[i], &attr, writemod, (void *)&worker[i]);
   };
  pthread_attr_destroy(&attr);

  for(i=0; i<NCPU; i++){
    pthread_join(MyThreads[i], &status);
  };



} else {

/* Case of one single process (NCPU=1). 
   Now, the master will do all the work. */

master.t0[cIF] = 0;
master.t1[cIF] = nt[cIF];
master.Iam = -1;

writemod((void *)&master);


};


  /* Add-up the Chi square, the error vector, and the  Hessian */
    for (i=0 ; i<NCPU; i++) {
      totChi += Chi2[i];
      for (j=0;j<npar;j++){
        Gradient[j] += WorkGrad[i][j];
      };
      for(j=0;j<nparsq;j++){
        Hessian[j] += WorkHess[i][j];
      };
    };




/* Update references and set the return value */

PyObject *ret = Py_BuildValue("d", totChi);


return ret;

}
