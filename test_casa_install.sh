
# Test the casa install by running a reduction script with test data included in CASA

cd $HOME

mkdir test_reduction
cd test_reduction

# Run the script (in non-interactive mode)
casa --no-logger -c ngc5921_demo.py False

# Add checks for certain files in the output
# TODO!!