#include <stdio.h>
#include "Lib.h"
#include "math/Matrix.h"
#include "math/Math.h"

#undef ASSERT
#define ASSERT(x) \
    if (!(x)) { \
        printf("Assertion failed: %s, file %s, line %d\\n", #x, __FILE__, __LINE__); \
        return 1; \
    }

int main() {
	idMath::Init();
    printf("Testing idlib_standalone...\\n");
    if (Swap_IsBigEndian()) {
        printf("Big endian machine.\\n");
    } else {
        printf("Little endian machine.\\n");
    }

    // Test CreateRotationZ
    idMat4 rotZ = idMat4::CreateRotationZ(idMath::PI / 2.0f);
    ASSERT(rotZ[0][0] == cos(idMath::PI / 2.0f));
    ASSERT(rotZ[0][1] == -sin(idMath::PI / 2.0f));
    ASSERT(rotZ[1][0] == sin(idMath::PI / 2.0f));
    ASSERT(rotZ[1][1] == cos(idMath::PI / 2.0f));
    printf("CreateRotationZ test passed.\\n");

    // Test CreateTranslation
    idMat4 trans = idMat4::CreateTranslation(1.0f, 2.0f, 3.0f);
    ASSERT(trans[0][3] == 1.0f);
    ASSERT(trans[1][3] == 2.0f);
    ASSERT(trans[2][3] == 3.0f);
    printf("CreateTranslation test passed.\\n");

    // Test CreateOrthographicOffCenter
    idMat4 ortho = idMat4::CreateOrthographicOffCenter(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
    ASSERT(ortho[0][0] == 1.0f);
    ASSERT(ortho[1][1] == 1.0f);
    ASSERT(ortho[2][2] == 0.5f);
    ASSERT(ortho[3][3] == 1.0f);
    printf("CreateOrthographicOffCenter test passed.\\n");

    // Test CreatePerspectiveFieldOfView
    idMat4 persp = idMat4::CreatePerspectiveFieldOfView(idMath::PI / 2.0f, 1.0f, 1.0f, 100.0f);
    ASSERT(persp[0][0] == 1.0f);
    ASSERT(persp[1][1] == 1.0f);
    ASSERT(persp[2][2] == -100.0f / 99.0f);
    ASSERT(persp[3][2] == -1.0f);
    printf("CreatePerspectiveFieldOfView test passed.\\n");

    // Test CreateLookAt
    idVec3 eye(0.0f, 0.0f, 5.0f);
    idVec3 target(0.0f, 0.0f, 0.0f);
    idVec3 up(0.0f, 1.0f, 0.0f);
    idMat4 lookAt = idMat4::CreateLookAt(eye, target, up);
    ASSERT(lookAt[0][0] == 1.0f);
    ASSERT(lookAt[1][1] == 1.0f);
    ASSERT(lookAt[2][2] == 1.0f);
    ASSERT(lookAt[0][3] == 0.0f);
    ASSERT(lookAt[1][3] == 0.0f);
    ASSERT(lookAt[2][3] == -5.0f);
    printf("CreateLookAt test passed.\\n");

    printf("All tests passed!\\n");

    return 0;
}
