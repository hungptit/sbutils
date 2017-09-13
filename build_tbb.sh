source ./get_build_options.sh

install_tbb() {
    pushd "$SRC_DIR/tbb"
    make clean

	# Customize some variables to make this build portable.
	make $BUILD_OPTS CXXFLAGS="-O3 -march=native" $BUILD_OPTS
	
	# Copy libraries to lib folder.
	rm -rf lib
	mkdir -p lib
    cp build/linux_intel64_gcc_cc*_release/*.so* lib/
    popd;
}

# Build tbb
install_tbb > /dev/null
