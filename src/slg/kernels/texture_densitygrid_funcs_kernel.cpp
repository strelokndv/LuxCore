#include <string>
namespace slg { namespace ocl {
std::string KernelSource_texture_densitygrid_funcs = 
"#line 2 \"densitygrid_funcs.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2016 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// DensityGrids support\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_ENABLE_TEX_DENSITYGRID) \n"
"\n"
"__global const void *DensityGrid_GetPixelsAddress(__global const float* restrict* restrict densityGridBuff,\n"
"		const uint page, const uint offset) {\n"
"	return &densityGridBuff[page][offset];\n"
"}\n"
"\n"
"\n"
"float DensityGrid_GetVoxel(__global const void *voxels,\n"
"						   const uint nx, const uint ny, const uint nz,\n"
"						   const int x, const  int y, const  int z) {\n"
"	const uint u = Mod(x, nx);\n"
"	const uint v = Mod(y, ny);\n"
"	const uint w = Mod(z, nz);\n"
"			\n"
"	const uint index = ((w * ny) + v) *nx + u;\n"
"\n"
"	const float a = ((__global const float *)voxels)[index];\n"
"\n"
"	return a;\n"
"}\n"
"\n"
"float DensityGridTexture_ConstEvaluateFloat(__global HitPoint *hitPoint, unsigned int densityGridIndex, __global const TextureMapping3D *mapping DENSITYGRIDS_PARAM_DECL) {\n"
"	\n"
"	__global const DensityGrid *densityGrid = &densityGridDescs[densityGridIndex];\n"
"	__global const void *voxels = DensityGrid_GetPixelsAddress(densityGridBuff, densityGrid->pageIndex, densityGrid->voxelsIndex);\n"
"\n"
"	const float3 P = TextureMapping3D_Map(mapping, hitPoint);\n"
"\n"
"	\n"
"	float x, y, z;\n"
"	int vx, vy, vz;\n"
"\n"
"	switch (densityGrid->wrapMode) {\n"
"		case WRAP_REPEAT:\n"
"			x = P.x * densityGrid->nx;\n"
"			vx = Floor2Int(x);\n"
"			x -= vx;\n"
"			vx = Mod(vx, densityGrid->nx);\n"
"			y = P.y * densityGrid->ny;\n"
"			vy = Floor2Int(y);\n"
"			y -= vy;\n"
"			vy = Mod(vy, densityGrid->ny);\n"
"			z = P.z * densityGrid->nz;\n"
"			vz = Floor2Int(z);\n"
"			z -= vz;\n"
"			vz = Mod(vz, densityGrid->nz);\n"
"			break;\n"
"		case WRAP_BLACK:\n"
"			if (P.x < 0.f || P.x >= 1.f ||\n"
"				P.y < 0.f || P.y >= 1.f ||\n"
"				P.z < 0.f || P.z >= 1.f)\n"
"				return 0.f;\n"
"			x = P.x * densityGrid->nx;\n"
"			vx = Floor2Int(x);\n"
"			x -= vx;\n"
"			y = P.y * densityGrid->ny;\n"
"			vy = Floor2Int(y);\n"
"			y -= vy;\n"
"			z = P.z * densityGrid->nz;\n"
"			vz = Floor2Int(z);\n"
"			z -= vz;\n"
"			break;\n"
"		case WRAP_WHITE:\n"
"			if (P.x < 0.f || P.x >= 1.f ||\n"
"				P.y < 0.f || P.y >= 1.f ||\n"
"				P.z < 0.f || P.z >= 1.f)\n"
"				return 1.f;\n"
"			x = P.x * densityGrid->nx;\n"
"			vx = Floor2Int(x);\n"
"			x -= vx;\n"
"			y = P.y * densityGrid->ny;\n"
"			vy = Floor2Int(y);\n"
"			y -= vy;\n"
"			z = P.z * densityGrid->nz;\n"
"			vz = Floor2Int(z);\n"
"			z -= vz;\n"
"			break;\n"
"		case WRAP_CLAMP:\n"
"			x = clamp(P.x, 0.f, 1.f) * densityGrid->nx;\n"
"			vx = min(Floor2Int(x), (int)densityGrid->nx - 1);\n"
"			x -= vx;\n"
"			y = clamp(P.y, 0.f, 1.f) * densityGrid->ny;\n"
"			vy = min(Floor2Int(P.y * densityGrid->ny), (int)densityGrid->ny - 1);\n"
"			y -= vy;\n"
"			z = clamp(P.z, 0.f, 1.f) * densityGrid->nz;\n"
"			vz = min(Floor2Int(P.z * densityGrid->nz), (int)densityGrid->nz - 1);\n"
"			z -= vz;\n"
"			break;\n"
"\n"
"		default:\n"
"			return 0.f;\n"
"	}\n"
"\n"
"	// Trilinear interpolation of the grid element\n"
"	return Lerp(z,\n"
"		Lerp(y, \n"
"			Lerp(x, DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx, vy, vz), DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx + 1, vy, vz)),\n"
"			Lerp(x, DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx, vy + 1, vz), DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx + 1, vy + 1, vz))),\n"
"		Lerp(y,\n"
"			Lerp(x, DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx, vy, vz + 1), DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx + 1, vy, vz + 1)),\n"
"			Lerp(x, DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx, vy + 1, vz + 1), DensityGrid_GetVoxel(voxels, densityGrid->nx, densityGrid->ny, densityGrid->nz, vx + 1, vy + 1, vz + 1))));\n"
"}\n"
"\n"
"float3 DensityGridTexture_ConstEvaluateSpectrum(__global HitPoint *hitPoint, unsigned int densityGridIndex, __global const TextureMapping3D *mapping DENSITYGRIDS_PARAM_DECL) {\n"
"	\n"
"	return  DensityGridTexture_ConstEvaluateFloat(hitPoint, densityGridIndex, mapping DENSITYGRIDS_PARAM);\n"
"}\n"
"\n"
"#endif\n"
; } }
