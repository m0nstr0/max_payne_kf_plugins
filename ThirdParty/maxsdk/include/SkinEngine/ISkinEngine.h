/**********************************************************************
*<
FILE: SkinEngine.h

DESCRIPTION:  Declaration of the SkinEngine

CREATED BY: Nikolai Sander

HISTORY: created 7/10/99
12/21/2015 Moved into Geom

*>	Copyright (c) 1999, All Rights Reserved.
**********************************************************************/

#pragma once

#include "SkinEngineExport.h"
#include "../geom/geom.h"
#include "../containers/Array.h"

#include <math.h>

class BoneModData;
class VertexListClass;
	
namespace MaxSDK
{
	namespace SkinEngine
	{
		template <class T>
		class StrideArray
		{
		protected:
			T* m_pT = nullptr;
			DWORD m_stride = 0;

		public:
			StrideArray(T* pT, DWORD stride)
				: m_pT(pT)
				, m_stride(stride)
			{
			}
			StrideArray() = default;

			StrideArray(const StrideArray&) = default;
			StrideArray(StrideArray&&) = default;
			StrideArray& operator=(const StrideArray&) = default;
			StrideArray& operator=(StrideArray&&) = default;

			T& operator[](const int i) const
			{ // access ith entry.
				assert(m_pT);
				return (*(T*)&((BYTE*)m_pT)[i * m_stride]);
			}
		};

		class CBoneDataClass
		{
		public:
			void SetInitTM(float *ptm);
			void SetInitTM(const Matrix3& ptm);
			void SetCurrentTM(const Matrix3& tm);
			const Matrix3& GetCurrentTM() const { return m_CurrentTM; }
			void SetFlags(DWORD flags);
			DWORD GetFlags() const { return m_flags; }

			void PreComputeXFormTM(const Matrix3& MeshTM);
			const Matrix3& GetXFormTM() const;
		protected:
			Matrix3 m_XFormTM;
			Matrix3 m_InitTM;
			Matrix3 m_CurrentTM;
			DWORD m_flags;
			BOOL bCacheValid;
		};



		class PointDataClass
		{
		public:
			int m_numBones;
			Point3 m_InitialPos;
			StrideArray<int> m_BoneIndices;
			StrideArray<float> m_BoneWeights;

			// This stuff is only needed for Spline bones interpolation
			StrideArray<int> m_SubCurveIndices;
			StrideArray<int> m_SubSegIndices;
			StrideArray<float> m_SubSegDistance;
			StrideArray<Point3> m_Tangents;
			StrideArray<Point3> m_OPoints;
		};


		/////////////////////////////////////////////////////////////////////////////
		// CSkinEngine
		class CSkinEngine : public MaxHeapOperators
		{

			// ISkinEngine
		public:
			SEEXPORT CSkinEngine();
			SEEXPORT virtual ~CSkinEngine();

			
			SEEXPORT STDMETHODIMP SetNumPoints(int numPoints);
			SEEXPORT STDMETHODIMP SetNumBones(int numBones);

			SEEXPORT STDMETHODIMP SetBoneFlags(int boneIdx, DWORD flags);

			SEEXPORT bool UseSplineAnimation();
			SEEXPORT void SetUseSplineAnimation(bool splineAnim);


			SEEXPORT STDMETHODIMP SetInitTM( float *InitTM);
			SEEXPORT STDMETHODIMP SetInitTM(const Matrix3& InitTM);

			SEEXPORT STDMETHODIMP SetInitBoneTM(int boneIdx,  float *InitTM);
			SEEXPORT STDMETHODIMP SetInitBoneTM(int boneIdx, const Matrix3& InitTM);
			
			SEEXPORT STDMETHODIMP SetBoneTM(int boneIdx, float *currentTM);
			SEEXPORT STDMETHODIMP SetBoneTM(int boneIdx, const Matrix3& currentTM);

			SEEXPORT STDMETHODIMP SetPointData(int pointIdx, int numData,
										DWORD b_stride, int *BoneIndexArray,
										DWORD w_stride, float *WeightArray,
										DWORD sci_stride, int *SubCurveIdxArray,
										DWORD ssi_stride, int *SubSegIdxArray,
										DWORD ssd_stride, float *SubSegDistArray,
										DWORD t_stride, float *TangentsArray,
										DWORD op_stride, float *OPointsArray);

			
			SEEXPORT STDMETHODIMP MapPoint(int idx, float *pin, float *pout) const;

			SEEXPORT virtual Point3 GetInterpCurvePiece3D(int BoneId, int CurveId, int SegId, float distance) const;
			SEEXPORT virtual Point3 GetTangentPiece3D(int BoneId, int CurveId, int SegId, float distance) const;

			SEEXPORT void PreComputeBoneXFormTMs();

		protected:
			Point3 SplineAnimation(int vertex, int bone, const Point3& p) const;


			MaxSDK::Array<CBoneDataClass> BoneData;
			MaxSDK::Array<PointDataClass> PointData;

			Matrix3 m_MeshTM;			
			bool mUseSplineAnimation;
		};

	}
}

