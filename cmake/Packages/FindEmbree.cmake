## ======================================================================== ##
## Copyright 2009-2014 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

FIND_PATH(EMBREE_INCLUDE_PATH NAMES embree2/rtcore.h PATHS
	${EMBREE_ROOT}/include
	/usr/include
	/usr/local/include
	/opt/local/include)
FIND_PATH(EMBREE_INCLUDE_PATH NAMES embree2/rtcore.h)

FIND_LIBRARY(EMBREE_LIBRARY NAMES embree libembree.so.2 PATHS
	${EMBREE_ROOT}/lib/x64
	${EMBREE_ROOT}/lib
	${EMBREE_ROOT}/build
	/usr/lib 
	/usr/lib64
	/usr/local/lib 
	/opt/local/lib NO_DEFAULT_PATH)
FIND_LIBRARY(EMBREE_LIBRARY NAMES embree libembree.so.2)

# Embree requires Intel TBB library
FIND_LIBRARY(TBB_LIBRARY NAMES tbb libtbb.so.2 PATHS
	${EMBREE_ROOT}/lib/x64
	${EMBREE_ROOT}/lib
	${EMBREE_ROOT}/build
	/usr/lib 
	/usr/lib64
	/usr/local/lib 
	/opt/local/lib NO_DEFAULT_PATH)
FIND_LIBRARY(TBB_LIBRARY NAMES tbb libtbb.so.2)

FIND_LIBRARY(TBBMALLOC_LIBRARY NAMES tbbmalloc libtbbmalloc.so.2 PATHS
	${EMBREE_ROOT}/lib/x64
	${EMBREE_ROOT}/lib
	${EMBREE_ROOT}/build
	/usr/lib 
	/usr/lib64
	/usr/local/lib 
	/opt/local/lib NO_DEFAULT_PATH)
FIND_LIBRARY(TBBMALLOC_LIBRARY NAMES tbbmalloc libtbbmalloc.so.2)


IF (EMBREE_INCLUDE_PATH AND EMBREE_LIBRARY AND TBB_LIBRARY AND TBBMALLOC_LIBRARY)
	SET(EMBREE_LIBRARY ${EMBREE_LIBRARY} ${TBB_LIBRARY} ${TBBMALLOC_LIBRARY})
	SET(EMBREE_FOUND TRUE)
ENDIF()

MARK_AS_ADVANCED(
	EMBREE_INCLUDE_PATH
	EMBREE_LIBRARY
)
