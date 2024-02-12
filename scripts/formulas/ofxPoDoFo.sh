#!/usr/bin/env bash
#

FORMULA_TYPES=( "osx" )

FORMULA_DEPENDS=()

# define the version
VER=0.10.3


# tools for git use
GIT_URL=https://github.com/podofo/podofo
GIT_TAG=0.10.3
GIT_HUB_URL=https://github.com/podofo/podofo/archive/refs/tags/0.10.3.tar.gz



# download the source code and unpack it into LIB_NAME
function download() {
	echo "Downloading ofxPoDoFo-$VER"

	. "$DOWNLOADER_SCRIPT"
	downloader $GIT_HUB_URL
	tar -xzf $VER.tar.gz
	mv podofo-$VER ofxPoDoFo
	rm $VER*.tar.gz
}

# prepare the build environment, executed inside the lib src dir
function prepare() {
	mkdir -p lib/$TYPE

	apothecaryDependencies download
    
    apothecaryDepend prepare zlib
    apothecaryDepend build zlib
    apothecaryDepend copy zlib
}

# executed inside the lib src dir
function build() {

	LIBS_ROOT=$(realpath $REAL_LIBS_DIR)
	echo "libs dir:$LIBS_DIR in addons"
	if [ "$TYPE" == "osx" ] ; then
		
		mkdir -p "build_${TYPE}_${PLATFORM}"
		cd "build_${TYPE}_${PLATFORM}"
		rm -f CMakeCache.txt *.a *.o

		ZLIB_ROOT="$LIBS_ROOT/zlib/"
		ZLIB_INCLUDE_DIR="$LIBS_ROOT/zlib/include"
		ZLIB_LIBRARY="$LIBS_ROOT/zlib/$TYPE/$PLATFORM/zlib.a"

		LIBPNG_ROOT="$LIBS_ROOT/libpng/"
        LIBPNG_INCLUDE_DIR="$LIBS_ROOT/libpng/include"
        LIBPNG_LIBRARY="$LIBS_ROOT/libpng/lib/$TYPE/$PLATFORM/libpng.a" 

        LIBSSL_ROOT="$LIBS_ROOT/openssl/"
        LIBSSL_INCLUDE_DIR="$LIBS_ROOT/openssl/include"
        LIBSSL_LIBRARY="$LIBS_ROOT/openssl/lib/$TYPE/$PLATFORM/libopenssl.a" 

        LIBXML2_ROOT="$LIBS_ROOT/libxml2/"
        LIBXML2_INCLUDE_DIR="$LIBS_ROOT/libxml2/include"
        LIBXML2_LIBRARY="$LIBS_ROOT/libxml2/lib/$TYPE/$PLATFORM/libxml2.a" 

        LIBFREETYPE_ROOT="$LIBS_ROOT/freetype/"
        LIBFREETYPE_INCLUDE_DIR="$LIBS_ROOT/freetype/include"
        LIBFREETYPE_LIBRARY="$LIBS_ROOT/freetype/lib/$TYPE/$PLATFORM/libfreetype.a" 

		DEFS="-DCMAKE_BUILD_TYPE=Release \
		    -DCMAKE_C_STANDARD=17 \
		    -DCMAKE_CXX_STANDARD=17 \
		    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
		    -DCMAKE_CXX_EXTENSIONS=OFF \
		    -DZLIB_ROOT=${ZLIB_ROOT} \
		    -DZLIB_INCLUDE_DIRS=$LIBS_ROOT/zlib/include \
		    -DZLIB_LIBRARY=$LIBS_ROOT/zlib/$TYPE/$PLATFORM/zlib.a \
		    -DLIBXML2_INCLUDE_DIRS=$LIBS_ROOT/libxml2 \
		    -DLIBXML2_LIBRARY=${LIBXML2_LIBRARY} \
		    -DLIBSSL_ROOT=${LIBSSL_ROOT} \
		    -DLIBSSL_INCLUDE_DIRS=${LIBSSL_INCLUDE_DIR} \
		    -DLIBSSL_LIBRARY=${LIBSSL_LIBRARY}/lib/libopenssl.a \ \
		    -DFREETYPE_INCLUDE_DIRS=${LIBFREETYPE_INCLUDE_DIR} \
		    -DFREETYPE_LIBRARY=${LIBFREETYPE_LIBRARY} \
		    -DFREETYPE_INCLUDE_DIR_freetype=${LIBFREETYPE_INCLUDE_DIR} \
            -DPNG_INCLUDE_DIR=${LIBPNG_INCLUDE_DIR} \
            -DPNG_LIBRARY=${LIBPNG_ROOT} \
            -DPNG_ROOT=${LIBPNG_ROOT} \
            -DCMAKE_FIND_FRAMEWORK=NEVER -DCMAKE_PREFIX_PATH=${LIBS_ROOT} \
            -DFontconfig_INCLUDE_DIR=`brew --prefix fontconfig`/include \
            -DOPENSSL_ROOT_DIR=${LIBSSL_ROOT} \
		    -DBUILD_SHARED_LIBS=OFF \
		    -DENABLE_STATIC=ON "

			cmake .. ${DEFS} \
				-DCMAKE_TOOLCHAIN_FILE=$APOTHECARY_DIR/ios.toolchain.cmake \
				-DCMAKE_CXX_FLAGS="-fvisibility-inlines-hidden -stdlib=libc++ -fPIC " \
		        -DCMAKE_C_FLAGS="-fvisibility-inlines-hidden -stdlib=libc++ -fPIC" \
		        -DCMAKE_CXX_FLAGS_RELEASE="-DUSE_PTHREADS=1 ${FLAG_RELEASE} " \
		        -DCMAKE_C_FLAGS_RELEASE="-DUSE_PTHREADS=1 ${FLAG_RELEASE} " \
		        -DCMAKE_INSTALL_PREFIX=install \
		        -DCMAKE_INSTALL_INCLUDEDIR=include \
		        -DCMAKE_INSTALL_LIBDIR="lib" \
		        -DCMAKE_INCLUDE_OUTPUT_DIRECTORY=include \
				-DPLATFORM=$PLATFORM \
				-DENABLE_BITCODE=OFF \
				-DPODOFO_BUILD_LIB_ONLY=TRUE \
				-DENABLE_ARC=OFF \
				-DENABLE_VISIBILITY=OFF \
				-DPODOFO_BUILD_STATIC=ON \
				-DCMAKE_VERBOSE_MAKEFILE=${VERBOSE_MAKEFILE} \
            	-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
					
		cmake --build . --config Release --target install
		cd ..	
	fi
}

# executed inside the lib src dir, first arg $1 is the dest libs dir root
function copy() {
    #remove old include files if they exist
    if [ -d "$1/include" ]; then
        rm -rf $1/include
    fi
	# copy headers
	mkdir -p $1/include/ofxPoDoFo/

	mkdir -p $1/lib/$TYPE
	if [[ "$TYPE" =~ ^(osx|ios|tvos|xros|catos|watchos)$ ]]; then
		mkdir -p $1/lib/$TYPE/$PLATFORM/
		cp -R "build_${TYPE}_${PLATFORM}/install/include/" $1/include
		cp -v "build_${TYPE}_${PLATFORM}/install/lib/libpodofo.a" $1/lib/$TYPE/$PLATFORM/libpodofo.a
		cp -v "build_${TYPE}_${PLATFORM}/install/lib/libpodofo_private.a" $1/lib/$TYPE/$PLATFORM/libpodofo_private.a
	fi

	# copy license files
	if [ -d "$1/license" ]; then
        rm -rf $1/license
    fi
	mkdir -p $1/license
	cp -v COPYING $1/license/LICENSE
}

# executed inside the lib src dir
function clean() {

	if [[ "$TYPE" =~ ^(osx|ios|tvos|xros|catos|watchos)$ ]]; then
		if [ -d "build_${TYPE}_${PLATFORM}" ]; then
			rm -r build_${TYPE}_${PLATFORM}     
		fi
	fi
}

function save() {
    . "$SAVE_SCRIPT" 
    savestatus ${TYPE} "ofxPoDoFo" ${ARCH} ${VER} true "${SAVE_FILE}"
}

function load() {
    . "$LOAD_SCRIPT"
    if loadsave ${TYPE} "ofxPoDoFo" ${ARCH} ${VER} "${SAVE_FILE}"; then
      return 0;
    else
      return 1;
    fi
}
