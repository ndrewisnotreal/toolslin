#pragma once
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct FVector {
    double X, Y, Z;
    FVector() : X(0), Y(0), Z(0) {}
    FVector(double x, double y, double z) : X(x), Y(y), Z(z) {}

    FVector operator+(const FVector& v) const { return FVector(X + v.X, Y + v.Y, Z + v.Z); }
    FVector operator-(const FVector& v) const { return FVector(X - v.X, Y - v.Y, Z - v.Z); }
    FVector operator*(double scalar) const { return FVector(X * scalar, Y * scalar, Z * scalar); }
    double Size() const { return std::sqrt(X * X + Y * Y + Z * Z); }
};

struct FRotator {
    double Pitch, Yaw, Roll;
    FRotator() : Pitch(0), Yaw(0), Roll(0) {}
    FRotator(double p, double y, double r) : Pitch(p), Yaw(y), Roll(r) {}

    FRotator operator+(const FRotator& v) const { return FRotator(Pitch + v.Pitch, Yaw + v.Yaw, Roll + v.Roll); }
    FRotator operator-(const FRotator& v) const { return FRotator(Pitch - v.Pitch, Yaw - v.Yaw, Roll - v.Roll); }
    FRotator operator*(double v) const { return FRotator(Pitch * v, Yaw * v, Roll * v); }

    void Normalize() {
        if (Pitch > 180) Pitch -= 360;
        else if (Pitch < -180) Pitch += 360;
        if (Yaw > 180) Yaw -= 360;
        else if (Yaw < -180) Yaw += 360;
    }
};

struct FVector4 { double X, Y, Z, W; };
struct FQuat { double X, Y, Z, W; };

struct D3DMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
};

struct FTransform {
    FQuat Rotation;
    FVector Translation;
    char pad_0x28[0x8];
    FVector Scale3D;
    char pad_0x48[0x8];

    D3DMATRIX ToMatrixWithScale() const;
};

struct FMatrix {
    double M[4][4];
};

struct FMinimalViewInfo {
    FVector Location;
    FRotator Rotation;
    float FOV;
};

D3DMATRIX MatrixMultiplication(const D3DMATRIX& pM1, const D3DMATRIX& pM2);
FMatrix RotatorToMatrix(const FRotator& rot);
bool WorldToScreen(const FVector& worldLoc, const FMinimalViewInfo& camera, int screenWidth, int screenHeight, FVector& outScreen);
FRotator CalcAngle(const FVector& src, const FVector& dst);
FVector RotationToVector(const FRotator& rot);
