#ifndef Engine_Function_h__
#define Engine_Function_h__

#include "Engine_Typedef.h"
#include <random>

namespace Engine
{
	// 템플릿은 기능의 정해져있으나 자료형은 정해져있지 않은 것
	// 기능을 인스턴스화 하기 위하여 만들어두는 틀

	template<typename T>
	void	Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void	Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned long Safe_Release(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)
		{
			dwRefCnt = pInstance->Release();

			if (0 == dwRefCnt)
				pInstance = nullptr;
		}

		return dwRefCnt;
	}

	// 레퍼카운트가 남아있더라도 연결을 강제 해제
	template<typename T>
	unsigned long Safe_ReleaseAndUnlink(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)
		{
			dwRefCnt = pInstance->Release();

			pInstance = nullptr;
		}

		return dwRefCnt;
	}

	template<typename T>
	unsigned long Safe_AddRef(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)		
			dwRefCnt = pInstance->AddRef();	

		return dwRefCnt;
	}

	static string WstrToStr(const wstring& wstr)
	{
		int length = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

		if (length > 0)
		{
			vector<char> utf8Buffer(length);
			if (WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8Buffer.data(), length, nullptr, nullptr) == 0)
				return string("");
			else
				return string(utf8Buffer.data());
		}
		return string("");
	}

	static 	wstring StrToWstr(const string& utf8Str)
	{
		int length = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);

		if (length > 0)
		{
			wstring wstr(length - 1, L'\0');

			MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], length - 1);
			return wstr;
		}
		return L"";
	}

	template<typename T, typename = enable_if<is_base_of<class CBase, T>::value>::type>
	constexpr shared_ptr<T> BaseMakeShared(T* pBase)
	{
		return shared_ptr<T>(pBase, [](T* pInstance) {
				pInstance->Release_Shared();
			});
	}

	static vector<wstring> SplitWstr(wstring strFull, _tchar cSeperator)
	{
		wstring temp;
		vector<wstring> parts;
		wstringstream wss(strFull);
		while (getline(wss, temp, cSeperator))
			parts.push_back(temp);

		return parts;
	}

	static vector<string> SplitStr(string strFull, _char cSeperator)
	{
		string temp;
		vector<string> parts;
		stringstream wss(strFull);
		while (getline(wss, temp, cSeperator))
			parts.push_back(temp);

		return parts;
	}

	static _bool Compare_Wstr(wstring strSoul, wstring strDest)
	{
		if (!wcscmp(strSoul.c_str(), strDest.c_str()))
			return true;

		return false;
	}

	static _bool Compare_Str(string strSoul, string strDest)
	{
		if (!strcmp(strSoul.c_str(), strDest.c_str()))
			return true;

		return false;
	}

	template<typename T>
	void Swap(T& lhs, T& rhs)
	{
		T temp = lhs;
		lhs = rhs;
		rhs = temp;
	}

	template <typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, T>::type
		Min(T value)
	{
		return value;
	}
	template <typename T, typename... Args>
	typename std::enable_if<std::is_arithmetic<T>::value, T>::type
		Min(T first, Args&&... args)
	{
		return min(first, Min(args...));
	}

	template <typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, T>::type
		Max(T value)
	{
		return value;
	}
	template <typename T, typename... Args>
	typename std::enable_if<std::is_arithmetic<T>::value, T>::type
		Max(T first, Args&&... args)
	{
		return max(first, Max(args...));
	}

	template<typename T>
	typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, _bool>::type
		InRange(T value, T low, T high, const string& range = "[)")
	{
		if (range == "()") {
			return value > low && value < high;
		}
		else if (range == "(]") {
			return value > low && value <= high;
		}
		else if (range == "[)") {
			return value >= low && value < high;
		}
		else if (range == "[]") {
			return value >= low && value <= high;
		}
		else
		{
			throw std::invalid_argument("Function::InRange: Invalid Range Option");
		}
	}

	template<typename T>
	T Random(initializer_list<T> _il)
	{
		assert(0 < _il.size());

		auto it = _il.begin();
		std::advance(it, rand() % _il.size());

		return *it;
	}

	template<typename T>
	typename std::enable_if<std::is_arithmetic<T>::value, T>::type
		Clamp(T low, T high, T value)
	{
		return min(max(value, low), high);
	}

	XMFLOAT3 Clamp(XMFLOAT3 low, XMFLOAT3 high, XMFLOAT3 value)
	{
		return XMFLOAT3(min(max(value.x, low.x), high.x), min(max(value.y, low.y), high.y), min(max(value.z, low.z), high.z));
	}
	XMFLOAT4 Clamp(XMFLOAT4 low, XMFLOAT4 high, XMFLOAT4 value)
	{
		return XMFLOAT4(min(max(value.x, low.x), high.x), min(max(value.y, low.y), high.y), min(max(value.z, low.z), high.z), min(max(value.w, low.w), high.w));
	}


#pragma region 문자열 함수
	/// <summary>
/// string to wstring 업그레이드 함수
/// </summary>
/// <param name="str">string</param>
/// <returns></returns>
	inline wstring ConvertToWstring(const string& str)
	{
		return wstring(str.begin(), str.end());
	}

	inline string ConvertToString(const wstring& str)
	{
		return string(str.begin(), str.end());
	}
#pragma endregion

#pragma region 컴파일러용 캐스팅 함수
	// static_cast
	template<typename Return, typename T>
	constexpr Return Cast(T value)
	{
		return static_cast<Return>(value);
	}

	// 다이나믹 캐스트
	template<typename Return, typename T>
	constexpr Return DynCast(T value)
	{
		return dynamic_cast<Return>(value);
	}

	// reinterpret_cast
	template<typename Return, typename T>
	constexpr Return ReCast(T value)
	{
		return reinterpret_cast<Return>(value);
	}

	template<typename Return, typename T>
	constexpr shared_ptr<Return> PtrCast(T value)
	{
		return static_pointer_cast<Return>(value);
	}

	template<typename Return, typename T>
	constexpr shared_ptr<Return> DynPtrCast(T value)
	{
		return dynamic_pointer_cast<Return>(value);
	}

	template<typename Return, typename T>
	constexpr shared_ptr<Return> RePtrCast(T value)
	{
		return reinterpret_pointer_cast<Return>(value);
	}

	// const_cast
	template<typename Return, typename T>
	constexpr Return ConCast(T value)
	{
		return const_cast<Return>(value);
	}

	// Default Enum 타입 변환기
	template<typename T, typename = enable_if_t<is_enum<T>::value>,
		typename Return = underlying_type_t<T>>
	constexpr Return ECast(T value)
	{
		return static_cast<Return>(value);
	}

	// void* 변환기
	template<typename T>
	constexpr void* VPCast(T value)
	{
		return static_cast<void*>(value);
	}

#pragma endregion
	 
#pragma region 디버깅용 행렬 요소 추출기
	// 행렬로부터 위치값 추출
	inline _float3 Get_PosFromMatrix(_float4x4 mat)
	{
		return _float3(mat._41, mat._42, mat._43);
	}

	// 행렬로부터 회전값 추출
	inline _float3 Get_RotEulerFromMatrix(_float4x4 mat)
	{
		_matrix Matrix = mat;

		_matrix matRot = {
			XMVector3Normalize(Matrix.r[0]),
			XMVector3Normalize(Matrix.r[1]),
			XMVector3Normalize(Matrix.r[2]),
			XMVectorSet(0.f, 0.f, 0.f, 1.f)
		};

		_vector vRot = XMQuaternionRotationMatrix(matRot);
		_float3 vEuler = {};
		_float pitch, yaw, roll;
		_float t0, t1;
		t0 = 2.f * (vRot.m128_f32[3] * vRot.m128_f32[0] + vRot.m128_f32[1] * vRot.m128_f32[2]);
		t1 = 1.f - 2.f * (vRot.m128_f32[0] * vRot.m128_f32[0] + vRot.m128_f32[1] * vRot.m128_f32[1]);
		pitch = atan2(t0, t1);

		_float t2;
		t2 = 2.f * (vRot.m128_f32[3] * vRot.m128_f32[1] - vRot.m128_f32[2] * vRot.m128_f32[0]);
		t2 = (t2 > 1.f) ? 1.f : t2;
		t2 = (t2 < -1.f) ? -1.f : t2;
		yaw = asin(t2);

		_float t3, t4;
		t3 = 2.f * (vRot.m128_f32[3] * vRot.m128_f32[2] + vRot.m128_f32[0] * vRot.m128_f32[1]);
		t4 = 1.f - 2.f * (vRot.m128_f32[1] * vRot.m128_f32[1] + vRot.m128_f32[2] * vRot.m128_f32[2]);
		roll = atan2(t3, t4);

		return _float3(XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll));
	}

	// 행렬로부터 크기값 추출
	inline _float3 Get_ScaleFromMatrix(_float4x4 mat)
	{
		_vector vLength[3] = {
			XMVectorSet(mat._11, mat._12, mat._13, 0.f),
			XMVectorSet(mat._21, mat._22, mat._23, 0.f),
			XMVectorSet(mat._31, mat._32, mat._33, 0.f)
		};

		return _float3(XMVector3Length(vLength[0]).m128_f32[0], XMVector3Length(vLength[1]).m128_f32[1], XMVector3Length(vLength[2]).m128_f32[2]);
	}


	inline XMMATRIX Get_RotationMatrix(FXMMATRIX Mat)
	{
		XMMATRIX ResultMat(XMMatrixIdentity());
		for (int i(0); i < 3; i++)
			ResultMat.r[i] = XMVector3Normalize(Mat.r[i]);

		return ResultMat;
	}

	inline XMMATRIX Get_ScaleMatrix(FXMMATRIX Mat)
	{
		XMMATRIX ResultMat(XMMatrixIdentity());
		for (int i(0); i < 3; i++)
			ResultMat.r[i].m128_f32[i] = XMVectorGetX(XMVector3Length(Mat.r[i]));

		return ResultMat;
	}

	inline XMMATRIX Get_PositionMatrix(FXMMATRIX Mat)
	{
		XMMATRIX ResultMat(XMMatrixIdentity());
		ResultMat.r[3] = Mat.r[3];
		return ResultMat;
	}

	inline XMMATRIX Get_MatrixNormalize(FXMMATRIX Mat)
	{
		_matrix ResultMat(XMMatrixIdentity());
		ResultMat.r[0] = XMVector3Normalize(Mat.r[0]);
		ResultMat.r[1] = XMVector3Normalize(Mat.r[1]);
		ResultMat.r[2] = XMVector3Normalize(Mat.r[2]);
		ResultMat.r[3] = Mat.r[3];
		return ResultMat;
	}

	inline XMFLOAT3 Extract_PitchYawRollFromRotationMatrix(FXMMATRIX Mat)
	{
		XMFLOAT4X4 MatFrom4x4;
		XMStoreFloat4x4(&MatFrom4x4, Mat);

		float pitch(DirectX::XMScalarASin(-MatFrom4x4._32));

		DirectX::XMVECTOR from(DirectX::XMVectorSet(MatFrom4x4._12, MatFrom4x4._31, 0.f, 0.f));
		DirectX::XMVECTOR to(DirectX::XMVectorSet(MatFrom4x4._22, MatFrom4x4._33, 0.f, 0.f));
		DirectX::XMVECTOR res(DirectX::XMVectorATan2(from, to));

		float roll(DirectX::XMVectorGetX(res));
		float yaw(DirectX::XMVectorGetY(res));

		return XMFLOAT3(pitch, yaw, roll);
	}

#pragma endregion

	inline float Distance(XMFLOAT3 v1, XMFLOAT3 v2)
	{
		const XMVECTOR x1 = XMLoadFloat3(&v1);
		const XMVECTOR x2 = XMLoadFloat3(&v2);
		const XMVECTOR V = XMVectorSubtract(x2, x1);
		const XMVECTOR X = XMVector3Length(V);
		return XMVectorGetX(X);
	}

	inline float Distance(XMFLOAT4 v1, XMFLOAT4 v2)
	{
		const XMVECTOR x1 = XMLoadFloat4(&v1);
		const XMVECTOR x2 = XMLoadFloat4(&v2);
		const XMVECTOR V = XMVectorSubtract(x2, x1);
		const XMVECTOR X = XMVector3Length(V);
		return XMVectorGetX(X);
	}

	inline float Distance(XMVECTOR v1, XMVECTOR v2)
	{
		const XMVECTOR V = XMVectorSubtract(v1, v2);
		const XMVECTOR X = XMVector3Length(V);
		return XMVectorGetX(X);
	}
}

#endif // Engine_Function_h__
