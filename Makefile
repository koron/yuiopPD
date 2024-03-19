default: build/Makefile
	cmake --build build --parallel 8

build/Makefile: CMakeLists.txt */CMakeLists.txt
	cmake -B build -DCMAKE_DEPENDS_USE_COMPILER=OFF

.PHONY: clean
clean:
	if [ -f build/Makefile ] ; then make -C build clean ; fi

.PHONY: distclean
distclean:
	rm -rf build
	rm -f tags

.PHONY: tags
tags:
	ctags --exclude=build/* --exclude=tmp/* -R .

.PHONY: install
install:
	make -j8 -C build
	cp ./build/rev1/yuiopPD_rev1.uf2 /e/
