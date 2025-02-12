
/*
	3D vector.
*/

#pragma once

class Vector3 
{
public:
	S3D_INLINE static const Vector3 Add(const Vector3 &A, const Vector3 &B) { return {A.x+B.x, A.y+B.y, A.z+B.z}; }
	S3D_INLINE static const Vector3 Sub(const Vector3 &A, const Vector3 &B) { return {A.x-B.x, A.y-B.y, A.z-B.z}; }
	S3D_INLINE static const Vector3 Mul(const Vector3 &A, const Vector3 &B) { return {A.x*B.x, A.y*B.y, A.z*B.z}; }
	S3D_INLINE static const Vector3 Div(const Vector3 &A, const Vector3 &B) { return {A.x/B.x, A.y/B.y, A.z/B.z}; }

	S3D_INLINE static const Vector3 Scale(const Vector3 &A, float B)
	{
		return { A.x*B, A.y*B, A.z*B };
	}

	S3D_INLINE static float Dot(const Vector3 &A, const Vector3 &B)
	{
		return A.x*B.x + A.y*B.y + A.z*B.z;
	}

	S3D_INLINE static const Vector3 Cross(const Vector3 &A, const Vector3 &B)
	{
		return Vector3(
			A.y*B.z - A.z*B.y,
			A.z*B.x - A.x*B.z,
			A.x*B.y - A.y*B.x);
	}

public:
	// 03/08/2018 - Added for Bevacqua.
	operator __m128() const { return vSSE; }

	union
	{
		struct {
			float x, y, z;
			float padding;
		};

		__m128 vSSE;
	};

	Vector3() {}
	
	explicit Vector3(float scalar) : 
		x(scalar), y(scalar), z(scalar), padding(0.f) {}

	Vector3(float x, float y, float z) :
		x(x), y(y), z(z), padding(0.f) {}

 	Vector3(const Vector2 &vec2D, float z = 1.f) :
		x(vec2D.x), y(vec2D.y), z(z), padding(0.f) {}

	explicit Vector3(__m128 _vSSE) :
		vSSE(_vSSE) {}

	const Vector3 operator +(const Vector3 &B) const { return Add(*this, B); }
	const Vector3 operator +(float B)          const { return Add(*this, Vector3(B)); }
	const Vector3 operator -(const Vector3 &B) const { return Sub(*this, B); }
	const Vector3 operator -(float B)          const { return Sub(*this, Vector3(B)); }
	const float   operator *(const Vector3 &B) const { return Dot(*this, B); }
	const Vector3 operator *(float B)          const { return Scale(*this, B); }
	const Vector3 operator /(const Vector3 &B) const { return Div(*this, B); }
	const Vector3 operator /(float B)          const { return Div(*this, Vector3(B)); }
	const Vector3 operator %(const Vector3 &B) const { return Cross(*this, B); }

	Vector3& operator +=(const Vector3 &B) { return *this = *this + B; }
	Vector3& operator +=(float B)          { return *this = *this + B; }
	Vector3& operator -=(const Vector3 &B) { return *this = *this - B; }
	Vector3& operator -=(float B)          { return *this = *this - B; }
	Vector3& operator *=(float B)          { return *this = *this * B; }
	Vector3& operator /=(const Vector3 &B) { return *this = *this / B; }
	Vector3& operator /=(float B)          { return *this = *this / B; }

	bool operator ==(const Vector3 &B) const
	{
		return comparef(x, B.x) && comparef(y, B.y) && comparef(z, B.z);
	}

	bool operator !=(const Vector3 &B) const
	{
		return false == (*this == B);
	}

	bool operator <(const Vector3 &B) const
	{
		return LengthSq() < B.LengthSq();
	}

	S3D_INLINE float LengthSq() const 
	{ 
		return Dot(*this, *this); 
	}

	S3D_INLINE float Length() const
	{
		return sqrtf(Dot(*this, *this));
	}
	
	const Vector3 Normalized() const
	{
		auto result = *this;
		result.Normalize();
		return result;
	}

	void Normalize()
	{
		const float length = Length();
		if (length > 0.f)
		{
			*this *= 1.f/length;
		}
	}

	const Vector3 Multiplied(const Vector3 &B) const
	{
		return Mul(*this, B);
	}

	void Multiply(const Vector3 &B)
	{
		*this = Mul(*this, B);
	}

	S3D_INLINE float Angle(const Vector3 &B) const
	{
		return acosf(Dot(*this, B));
	}

	const Vector3 Project(const Vector3 &B) const
	{
		// A1 = |A|*cosAng=A*(B/|B|)
		// A' = (B/|B|)*(A1)
		const Vector3 unitB = B.Normalized();
		return B.Normalized() * Dot(*this, unitB);
	}

	const Vector3 Reflect(const Vector3 &normal) const
	{		
		const float R = 2.f*Dot(*this, normal);
		return *this - normal*R;
	}

	// Refraction according to Snell's law.
	// Note that 'etaRatio' is the ratio between both surface refraction indices.
	const Vector3 Refract(const Vector3 &normal, float etaRatio) const
	{
		const Vector3 &incident = *this;
		const float dirCos = Dot(incident*-1.f, normal);
		const float cosT2 = 1.f - etaRatio*etaRatio*(1.f - dirCos*dirCos); 
		const Vector3 refracted = incident*etaRatio + normal*(etaRatio*dirCos - sqrtf(fabsf(cosT2)));
		return refracted * std::max<float>(0.f, cosT2);
	}

	// A few basic refraction indices.
	static const float kRefractVacuum;
	static const float kRefractAir;
	static const float kRefractWater;	
	static const float kRefractGlass;
	static const float kRefractPlastic;
	static const float kRefractDiamond;

	S3D_INLINE const Vector3 Perpendicular(const Vector3 &B) const
	{
		return Cross(*this, B);
	}
};
