#include "math.h"

static double SafeCameraFOV(double fov) {
    if (!std::isfinite(fov)) return 90.0;
    if (fov < 40.0) return 40.0;
    if (fov > 160.0) return 160.0;
    return fov;
}

FMatrix RotatorToMatrix(const FRotator& rot) {
    FMatrix m;
    double radPitch = rot.Pitch * (M_PI / 180.0);
    double radYaw = rot.Yaw * (M_PI / 180.0);
    double radRoll = rot.Roll * (M_PI / 180.0);

    double SP = sin(radPitch);
    double CP = cos(radPitch);
    double SY = sin(radYaw);
    double CY = cos(radYaw);
    double SR = sin(radRoll);
    double CR = cos(radRoll);

    m.M[0][0] = CP * CY;
    m.M[0][1] = CP * SY;
    m.M[0][2] = SP;
    m.M[0][3] = 0.0;

    m.M[1][0] = SR * SP * CY - CR * SY;
    m.M[1][1] = SR * SP * SY + CR * CY;
    m.M[1][2] = -SR * CP;
    m.M[1][3] = 0.0;

    m.M[2][0] = -(CR * SP * CY + SR * SY);
    m.M[2][1] = CY * SR - CR * SP * SY;
    m.M[2][2] = CR * CP;
    m.M[2][3] = 0.0;

    m.M[3][0] = 0.0;
    m.M[3][1] = 0.0;
    m.M[3][2] = 0.0;
    m.M[3][3] = 1.0;
    return m;
}

bool WorldToScreen(const FVector& worldLoc, const FMinimalViewInfo& camera, int screenWidth, int screenHeight, FVector& outScreen) {
    FMatrix tempMatrix = RotatorToMatrix(camera.Rotation);
    
    FVector vAxisX(tempMatrix.M[0][0], tempMatrix.M[0][1], tempMatrix.M[0][2]);
    FVector vAxisY(tempMatrix.M[1][0], tempMatrix.M[1][1], tempMatrix.M[1][2]);
    FVector vAxisZ(tempMatrix.M[2][0], tempMatrix.M[2][1], tempMatrix.M[2][2]);
    
    FVector vDelta(worldLoc.X - camera.Location.X, worldLoc.Y - camera.Location.Y, worldLoc.Z - camera.Location.Z);
    
    FVector vTransformed(
        vDelta.X * vAxisY.X + vDelta.Y * vAxisY.Y + vDelta.Z * vAxisY.Z,
        vDelta.X * vAxisZ.X + vDelta.Y * vAxisZ.Y + vDelta.Z * vAxisZ.Z,
        vDelta.X * vAxisX.X + vDelta.Y * vAxisX.Y + vDelta.Z * vAxisX.Z
    );
    
    if (vTransformed.Z < 1.0) return false; // Behind camera
    
    double screenCenterX = screenWidth / 2.0;
    double screenCenterY = screenHeight / 2.0;
    
    double focal = screenCenterX / tan(SafeCameraFOV(camera.FOV) * M_PI / 360.0);
    outScreen.X = screenCenterX + vTransformed.X * focal / vTransformed.Z;
    outScreen.Y = screenCenterY - vTransformed.Y * focal / vTransformed.Z;
    
    // Remove bounds check to allow unclipped projection (e.g. for large ESP boxes partially off-screen)
    return std::isfinite(outScreen.X) && std::isfinite(outScreen.Y);
}
D3DMATRIX FTransform::ToMatrixWithScale() const {
    D3DMATRIX m;
    m._41 = (float)Translation.X;
    m._42 = (float)Translation.Y;
    m._43 = (float)Translation.Z;

    float x2 = (float)(Rotation.X + Rotation.X);
    float y2 = (float)(Rotation.Y + Rotation.Y);
    float z2 = (float)(Rotation.Z + Rotation.Z);

    float xx2 = (float)(Rotation.X * x2);
    float yy2 = (float)(Rotation.Y * y2);
    float zz2 = (float)(Rotation.Z * z2);

    auto saneScale = [](double value) {
        if (!std::isfinite(value) || std::abs(value) < 0.001 || std::abs(value) > 100.0) {
            return 1.0f;
        }
        return (float)value;
    };

    float sX = saneScale(Scale3D.X);
    float sY = saneScale(Scale3D.Y);
    float sZ = saneScale(Scale3D.Z);

    m._11 = (1.0f - (yy2 + zz2)) * sX;
    m._22 = (1.0f - (xx2 + zz2)) * sY;
    m._33 = (1.0f - (xx2 + yy2)) * sZ;

    float yz2 = (float)(Rotation.Y * z2);
    float wx2 = (float)(Rotation.W * x2);
    m._32 = (yz2 - wx2) * sZ;
    m._23 = (yz2 + wx2) * sY;

    float xy2 = (float)(Rotation.X * y2);
    float wz2 = (float)(Rotation.W * z2);
    m._21 = (xy2 - wz2) * sY;
    m._12 = (xy2 + wz2) * sX;

    float xz2 = (float)(Rotation.X * z2);
    float wy2 = (float)(Rotation.W * y2);
    m._31 = (xz2 + wy2) * sZ;
    m._13 = (xz2 - wy2) * sX;

    m._14 = 0.0f;
    m._24 = 0.0f;
    m._34 = 0.0f;
    m._44 = 1.0f;

    return m;
}

D3DMATRIX MatrixMultiplication(const D3DMATRIX& pM1, const D3DMATRIX& pM2) {
    D3DMATRIX pOut;
    pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
    pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
    pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
    pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
    pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
    pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
    pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
    pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
    pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
    pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
    pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
    pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
    pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
    pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
    pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
    pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;
    return pOut;
}

FRotator CalcAngle(const FVector& src, const FVector& dst) {
    FVector diff = { dst.X - src.X, dst.Y - src.Y, dst.Z - src.Z };
    FRotator rot;
    rot.Pitch = std::atan2(diff.Z, std::sqrt(diff.X * diff.X + diff.Y * diff.Y)) * (180.0 / M_PI);
    rot.Yaw   = std::atan2(diff.Y, diff.X) * (180.0 / M_PI);
    rot.Roll  = 0.0;
    return rot;
}

FVector RotationToVector(const FRotator& rot) {
    double radPitch = rot.Pitch * (M_PI / 180.0);
    double radYaw   = rot.Yaw * (M_PI / 180.0);
    
    double CP = cos(radPitch);
    double SP = sin(radPitch);
    double CY = cos(radYaw);
    double SY = sin(radYaw);
    
    return FVector(CP * CY, CP * SY, SP);
}
