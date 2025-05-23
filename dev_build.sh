# run uninstall target
if [ -d "build" ]; then
	cd build
	make uninstall
	cd ..
	rm -rf build
fi

# manually remove rogue shared objects (not
# doing this step will not affect the build,
# but there may be runtime Python import issues)
rm -f /usr/local/lib/libgnuradio-xxx.so*

# build and install
mkdir build
cd build
cmake -DENABLE_SANDBOX=ON ..
make -j$(nproc)
make install
ldconfig