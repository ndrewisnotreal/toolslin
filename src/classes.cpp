#include "classes.h"
#include "memory.h"
#include <iostream>

GameOffsets GOffsets;

bool LoadOffsets(const char* filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << filepath << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;

        auto parse_hex = [&j](const std::string& key, const std::string& default_val) -> uintptr_t {
            std::string hex_str = j.value(key, default_val);
            try {
                return std::stoull(hex_str, nullptr, 16);
            } catch (...) {
                return 0;
            }
        };

        GOffsets.GWorldOffset = parse_hex("GWorldOffset", "0x0");
        GOffsets.GNameOffset = parse_hex("GNameOffset", "0x0");
        GOffsets.GObjectsOffset = parse_hex("GObjectsOffset", "0x0");
        GOffsets.GObjects_NumElementsOffset = parse_hex("GObjects_NumElementsOffset", "0x14");
        GOffsets.UWorld_PersistentLevelOffset = parse_hex("UWorld_PersistentLevelOffset", "0x50");
        GOffsets.UWorld_OwningGameInstanceOffset = parse_hex("UWorld_OwningGameInstanceOffset", "0x250");
        GOffsets.UWorld_GameStateOffset = parse_hex("UWorld_GameStateOffset", "0x1D8");
        GOffsets.ULevel_ActorsOffset = parse_hex("ULevel_ActorsOffset", "0xC0");
        GOffsets.Actors_NumElementsOffset = parse_hex("Actors_NumElementsOffset", "0x8");
        GOffsets.UObject_FNameOffset = parse_hex("UObject_FNameOffset", "0x18");
        GOffsets.UObject_ClassOffset = parse_hex("UObject_ClassOffset", "0x10");
        GOffsets.UObject_OuterOffset = parse_hex("UObject_OuterOffset", "0x28");
        GOffsets.FName_ComparisonIndexOffset = parse_hex("FName_ComparisonIndexOffset", "0x0");
        GOffsets.FName_NumberOffset = parse_hex("FName_NumberOffset", "0x4");
        GOffsets.FName_DisplayIndexOffset = parse_hex("FName_DisplayIndexOffset", "0x8");
        GOffsets.UGameInstance_LocalPlayersOffset = parse_hex("UGameInstance_LocalPlayersOffset", "0x58");
        GOffsets.UPlayer_PlayerControllerOffset = parse_hex("UPlayer_PlayerControllerOffset", "0x50");
        GOffsets.APlayerController_PlayerCameraManagerOffset = parse_hex("APlayerController_PlayerCameraManagerOffset", "0x3b0");
        GOffsets.APlayerController_AcknowledgedPawnOffset = parse_hex("APlayerController_AcknowledgedPawnOffset", "0x3a0");
        GOffsets.APawn_ControllerOffset = parse_hex("APawn_ControllerOffset", "0x320");
        GOffsets.APawn_PlayerStateOffset = parse_hex("APawn_PlayerStateOffset", "0x310");
        GOffsets.ACharacter_MeshOffset = parse_hex("ACharacter_MeshOffset", "0x370");
        GOffsets.ACharacter_CapsuleComponentOffset = parse_hex("ACharacter_CapsuleComponentOffset", "0x380");
        GOffsets.ACharacter_CharacterMovementOffset = parse_hex("ACharacter_CharacterMovementOffset", "0x378");
        GOffsets.UMovementComponent_VelocityOffset = parse_hex("UMovementComponent_VelocityOffset", "0x100");
        GOffsets.UCapsuleComponent_CapsuleHalfHeightOffset = parse_hex("UCapsuleComponent_CapsuleHalfHeightOffset", "0x598");
        GOffsets.ADBDPlayer_CameraOffset = parse_hex("ADBDPlayer_CameraOffset", "0xA58");
        GOffsets.UCameraComponent_FieldOfViewOffset = parse_hex("UCameraComponent_FieldOfViewOffset", "0x280");
        GOffsets.AActor_RootComponentOffset = parse_hex("AActor_RootComponentOffset", "0x1E0");
        GOffsets.USceneComponent_LocationOffset = parse_hex("USceneComponent_LocationOffset", "0x178");
        GOffsets.APlayerCameraManager_CameraCachePrivateOffset = parse_hex("APlayerCameraManager_CameraCachePrivateOffset", "0x15A0");
        GOffsets.APlayerCameraManager_DefaultFOVOffset = parse_hex("APlayerCameraManager_DefaultFOVOffset", "0x30c");
        GOffsets.FCameraCacheEntry_POVOffset = parse_hex("FCameraCacheEntry_POVOffset", "0x10");
        GOffsets.FMinimalViewInfo_LocationOffset = parse_hex("FMinimalViewInfo_LocationOffset", "0x0");
        GOffsets.FMinimalViewInfo_RotationOffset = parse_hex("FMinimalViewInfo_RotationOffset", "0x18");
        GOffsets.FMinimalViewInfo_FOVOffset = parse_hex("FMinimalViewInfo_FOVOffset", "0x30");
        GOffsets.APlayerState_PlayerNamePrivateOffset = parse_hex("APlayerState_PlayerNamePrivateOffset", "0x390");
        GOffsets.ASurvivor__healthComponentOffset = parse_hex("ASurvivor__healthComponentOffset", "0x1868");
        GOffsets.UHealthComponent__currentHealthStateCountOffset = parse_hex("UHealthComponent__currentHealthStateCountOffset", "0x2C8");
        GOffsets.ADBDPlayerState__inParadiseOffset = parse_hex("ADBDPlayerState__inParadiseOffset", "0x530");
        GOffsets.ADBDPlayerState_playerDataOffset = parse_hex("ADBDPlayerState_playerDataOffset", "0x570");
        GOffsets.ADBDPlayerState_GameRoleOffset = parse_hex("ADBDPlayerState_GameRoleOffset", "0x3DA");
        GOffsets.FPlayerStateData_EquippedPerkIdsOffset = parse_hex("FPlayerStateData_EquippedPerkIdsOffset", "0x10");
        GOffsets.FPlayerStateData_EquippedFavorIdOffset = parse_hex("FPlayerStateData_EquippedFavorIdOffset", "0x04");
        GOffsets.FPlayerStateData_powerOrItemIdOffset = parse_hex("FPlayerStateData_powerOrItemIdOffset", "0x5C");
        GOffsets.FPlayerStateData_addonIdsOffset = parse_hex("FPlayerStateData_addonIdsOffset", "0x68");
        GOffsets.AGameStateBase_PlayerArrayOffset = parse_hex("AGameStateBase_PlayerArrayOffset", "0x308");
        GOffsets.AController_PlayerStateOffset = parse_hex("AController_PlayerStateOffset", "0x240");
        GOffsets.APallet__stateOffset = parse_hex("APallet__stateOffset", "0x568");
        GOffsets.AWindow__isBlockedByLevelOffset = parse_hex("AWindow__isBlockedByLevelOffset", "0x438");
        GOffsets.ATotem__totemStateOffset = parse_hex("ATotem__totemStateOffset", "0x450");
        GOffsets.AGenerator__activatedOffset = parse_hex("AGenerator__activatedOffset", "0x6E8");
        GOffsets.AGenerator__generatorChargeableOffset = parse_hex("AGenerator__generatorChargeableOffset", "0x508");
        GOffsets.UChargeableComponent__currentChargeOffset = parse_hex("UChargeableComponent__currentChargeOffset", "0x1C8");
        GOffsets.FSpeedBasedNetSyncedValue__replicatedValueOffset = parse_hex("FSpeedBasedNetSyncedValue__replicatedValueOffset", "0x18");
        GOffsets.AGenerator__generatorChargeInSecondsOffset = parse_hex("AGenerator__generatorChargeInSecondsOffset", "0x688");
        GOffsets.FDBDTunableRowHandle__defaultValueOffset = parse_hex("FDBDTunableRowHandle__defaultValueOffset", "0x18");
        GOffsets.AHatch__hatchStateOffset = parse_hex("AHatch__hatchStateOffset", "0x460");
        GOffsets.ABaseTrap__isTrapSetOffset = parse_hex("ABaseTrap__isTrapSetOffset", "0x5C0");
        GOffsets.ALocker__playerInLockerOffset = parse_hex("ALocker__playerInLockerOffset", "0x4A0");
        GOffsets.BreakableBase__stateOffset = parse_hex("BreakableBase__stateOffset", "0x418");
        GOffsets.DBDAuraComponent_ColorOffset = parse_hex("DBDAuraComponent_ColorOffset", "0x374");
        GOffsets.DBDAuraComponent_MinimumDistanceWhenIsAlwaysVisibleOffset = parse_hex("DBDAuraComponent_MinimumDistanceWhenIsAlwaysVisibleOffset", "0x31C");
        GOffsets.DBDAuraComponent_MinimumDistanceOffset = parse_hex("DBDAuraComponent_MinimumDistanceOffset", "0x320");
        GOffsets.DBDAuraComponent_IsAlwaysVisibleOffset = parse_hex("DBDAuraComponent_IsAlwaysVisibleOffset", "0x318");
        GOffsets.DBDGameState__gameLevelEndedOffset = parse_hex("DBDGameState__gameLevelEndedOffset", "0x7B1");
        GOffsets.DBDGameState__gameTimedOut = parse_hex("DBDGameState__gameTimedOut", "0x7B2");
        GOffsets.DBDGameState__gameLevelCreated = parse_hex("DBDGameState__gameLevelCreated", "0x7B0");
        GOffsets.EscapeDoor__activatedOffset = parse_hex("EscapeDoor__activatedOffset", "0x460");
        GOffsets.UStruct_SuperStructOffset = parse_hex("UStruct_SuperStructOffset", "0x60");
        GOffsets.RedStainComponent__initialSpotLightColorOffset = parse_hex("RedStainComponent__initialSpotLightColorOffset", "0x4F0");
        GOffsets.AKiller_attackerComponent = parse_hex("AKiller_attackerComponent", "0x1A58");
        GOffsets.UDBDAttackerComponent_requestedAttack = parse_hex("UDBDAttackerComponent_requestedAttack", "0x188");
        GOffsets.UDBDAttackerComponent_currentAttack = parse_hex("UDBDAttackerComponent_currentAttack", "0x190");
        GOffsets.ADBDPlayer_interactionHandler = parse_hex("ADBDPlayer_interactionHandler", "0xB60");
        GOffsets.UPlayerInteractionHandler_skillCheck = parse_hex("UPlayerInteractionHandler_skillCheck", "0x358");
        GOffsets.UPlayerInteractionHandler_currentInteraction = parse_hex("UPlayerInteractionHandler_currentInteraction", "0x258");
        GOffsets.UPlayerInteractionHandler_interactionInProgress = parse_hex("UPlayerInteractionHandler_interactionInProgress", "0x409");
        GOffsets.UInteractionDefinition_interactionName = parse_hex("UInteractionDefinition_interactionName", "0x40");
        GOffsets.ADBDPlayer_chaseComponent = parse_hex("ADBDPlayer_chaseComponent", "0x0E48");
        GOffsets.UChaseComponent_isInChase = parse_hex("UChaseComponent_isInChase", "0x0128");
        GOffsets.UHealthComponent_healFromDyingChargeable = parse_hex("UHealthComponent_healFromDyingChargeable", "0x0308");
        GOffsets.ASurvivor_carriedMovementComponent = parse_hex("ASurvivor_carriedMovementComponent", "0x19E0");
        GOffsets.USkillCheck_isDisplayed = parse_hex("USkillCheck_isDisplayed", "0x1A8");
        GOffsets.USkillCheck_currentProgress = parse_hex("USkillCheck_currentProgress", "0x1AC");
        GOffsets.USkillCheck_customType = parse_hex("USkillCheck_customType", "0x1D8");
        GOffsets.USkillCheck_skillCheckDefinition = parse_hex("USkillCheck_skillCheckDefinition", "0x208");
        GOffsets.USkinnedMeshComponent_SkeletalMesh = parse_hex("USkinnedMeshComponent_SkeletalMesh", "0x628");
        GOffsets.USkinnedMeshComponent_BoneArray = parse_hex("USkinnedMeshComponent_BoneArray", "0x6A8");
        GOffsets.USkinnedMeshComponent_ComponentToWorld = parse_hex("USkinnedMeshComponent_ComponentToWorld", "0x220");
        GOffsets.USkeletalMesh_RawRefBoneInfo = parse_hex("USkeletalMesh_RawRefBoneInfo", "0x360");
        GOffsets.APlayerState_CompressedPingOffset = parse_hex("APlayerState_CompressedPingOffset", "0x02F8");
        GOffsets.ADBDPlayerState_platformAccountIdOffset = parse_hex("ADBDPlayerState_platformAccountIdOffset", "0x0640");
        GOffsets.ADBDPlayerState_platformOffset = parse_hex("ADBDPlayerState_platformOffset", "0x07AC");
        GOffsets.ADBDPlayerState_providerOffset = parse_hex("ADBDPlayerState_providerOffset", "0x07B0");
        GOffsets.ADBDPlayerState_crossplayAllowedOffset = parse_hex("ADBDPlayerState_crossplayAllowedOffset", "0x07B4");
        GOffsets.ADBDPlayerState_survivorPipsOffset = parse_hex("ADBDPlayerState_survivorPipsOffset", "0x07D8");
        GOffsets.ADBDPlayerState_killerPipsOffset = parse_hex("ADBDPlayerState_killerPipsOffset", "0x07DC");
        GOffsets.ADBDPlayerState_pktLossPercentageOffset = parse_hex("ADBDPlayerState_pktLossPercentageOffset", "0x07F8");
        GOffsets.FPlayerStateData_CharacterLevelOffset = parse_hex("FPlayerStateData_CharacterLevelOffset", "0x00");
        GOffsets.FPlayerStateData_prestigeLevelOffset = parse_hex("FPlayerStateData_prestigeLevelOffset", "0x7C");
        GOffsets.AGameStateBase_ReplicatedWorldTimeSecondsOffset = parse_hex("AGameStateBase_ReplicatedWorldTimeSecondsOffset", "0x031C");
        GOffsets.AGameStateBase_ReplicatedWorldTimeSecondsDoubleOffset = parse_hex("AGameStateBase_ReplicatedWorldTimeSecondsDoubleOffset", "0x0320");
        GOffsets.FSpeedBasedNetSyncedValue_replicatedSpeedOffset = parse_hex("FSpeedBasedNetSyncedValue_replicatedSpeedOffset", "0x20");
        GOffsets.FSpeedBasedNetSyncedValue_replicatedLastUpdateTimestampOffset = parse_hex("FSpeedBasedNetSyncedValue_replicatedLastUpdateTimestampOffset", "0x28");

        std::cout << "[INFO] GWorldOffset 0x" << std::hex << GOffsets.GWorldOffset << std::dec << std::endl;
        std::cout << "[INFO] GNameOffset 0x" << std::hex << GOffsets.GNameOffset << std::dec << std::endl;
        std::cout << "[INFO] GObjectsOffset 0x" << std::hex << GOffsets.GObjectsOffset << std::dec << std::endl;
        std::cout << "[INFO] UWorld_PersistentLevel 0x" << std::hex << GOffsets.UWorld_PersistentLevelOffset << std::dec << std::endl;
        std::cout << "[INFO] UWorld_OwningGameInstance 0x" << std::hex << GOffsets.UWorld_OwningGameInstanceOffset << std::dec << std::endl;
        std::cout << "[INFO] UWorld_GameState 0x" << std::hex << GOffsets.UWorld_GameStateOffset << std::dec << std::endl;
        std::cout << "[INFO] ULevel_Actors 0x" << std::hex << GOffsets.ULevel_ActorsOffset << std::dec << std::endl;
        std::cout << "[INFO] Actors_NumElements 0x" << std::hex << GOffsets.Actors_NumElementsOffset << std::dec << std::endl;
        std::cout << "[INFO] UObject_FName 0x" << std::hex << GOffsets.UObject_FNameOffset << std::dec << std::endl;
        std::cout << "[INFO] UObject_Class 0x" << std::hex << GOffsets.UObject_ClassOffset << std::dec << std::endl;
        std::cout << "[INFO] FName_ComparisonIndex 0x" << std::hex << GOffsets.FName_ComparisonIndexOffset << std::dec << std::endl;
        std::cout << "[INFO] FName_Number 0x" << std::hex << GOffsets.FName_NumberOffset << std::dec << std::endl;
        std::cout << "[INFO] FName_DisplayIndex 0x" << std::hex << GOffsets.FName_DisplayIndexOffset << std::dec << std::endl;
        std::cout << "[INFO] UGameInstance_LocalPlayers 0x" << std::hex << GOffsets.UGameInstance_LocalPlayersOffset << std::dec << std::endl;
        std::cout << "[INFO] UPlayer_PlayerController 0x" << std::hex << GOffsets.UPlayer_PlayerControllerOffset << std::dec << std::endl;
        std::cout << "[INFO] APlayerController_PlayerCameraManager 0x" << std::hex << GOffsets.APlayerController_PlayerCameraManagerOffset << std::dec << std::endl;
        std::cout << "[INFO] APlayerController_AcknowledgedPawn 0x" << std::hex << GOffsets.APlayerController_AcknowledgedPawnOffset << std::dec << std::endl;
        std::cout << "[INFO] APawn_Controller 0x" << std::hex << GOffsets.APawn_ControllerOffset << std::dec << std::endl;
        std::cout << "[INFO] APawn_PlayerState 0x" << std::hex << GOffsets.APawn_PlayerStateOffset << std::dec << std::endl;
        std::cout << "[INFO] ADBDPlayer_Camera 0x" << std::hex << GOffsets.ADBDPlayer_CameraOffset << std::dec << std::endl;
        std::cout << "[INFO] UCameraComponent_FieldOfView 0x" << std::hex << GOffsets.UCameraComponent_FieldOfViewOffset << std::dec << std::endl;
        std::cout << "[INFO] AActor_RootComponent 0x" << std::hex << GOffsets.AActor_RootComponentOffset << std::dec << std::endl;
        std::cout << "[INFO] USceneComponent_Location 0x" << std::hex << GOffsets.USceneComponent_LocationOffset << std::dec << std::endl;
        std::cout << "[INFO] APlayerCameraManager_CameraCachePrivate 0x" << std::hex << GOffsets.APlayerCameraManager_CameraCachePrivateOffset << std::dec << std::endl;
        std::cout << "[INFO] FCameraCacheEntry_POV 0x" << std::hex << GOffsets.FCameraCacheEntry_POVOffset << std::dec << std::endl;
        std::cout << "[INFO] FMinimalViewInfo_Location 0x" << std::hex << GOffsets.FMinimalViewInfo_LocationOffset << std::dec << std::endl;
        std::cout << "[INFO] FMinimalViewInfo_Rotation 0x" << std::hex << GOffsets.FMinimalViewInfo_RotationOffset << std::dec << std::endl;
        std::cout << "[INFO] FMinimalViewInfo_FOV 0x" << std::hex << GOffsets.FMinimalViewInfo_FOVOffset << std::dec << std::endl;
        std::cout << "[INFO] APlayerState_PlayerNamePrivate 0x" << std::hex << GOffsets.APlayerState_PlayerNamePrivateOffset << std::dec << std::endl;
        std::cout << "[INFO] ASurvivor__healthComponent 0x" << std::hex << GOffsets.ASurvivor__healthComponentOffset << std::dec << std::endl;
        std::cout << "[INFO] UHealthComponent__currentHealthStateCount 0x" << std::hex << GOffsets.UHealthComponent__currentHealthStateCountOffset << std::dec << std::endl;
        std::cout << "[INFO] ADBDPlayerState__inParadise 0x" << std::hex << GOffsets.ADBDPlayerState__inParadiseOffset << std::dec << std::endl;
        std::cout << "[INFO] ADBDPlayerState_playerData 0x" << std::hex << GOffsets.ADBDPlayerState_playerDataOffset << std::dec << std::endl;
        std::cout << "[INFO] FPlayerStateData_EquippedPerkIds 0x" << std::hex << GOffsets.FPlayerStateData_EquippedPerkIdsOffset << std::dec << std::endl;
        std::cout << "[INFO] APallet__state 0x" << std::hex << GOffsets.APallet__stateOffset << std::dec << std::endl;
        std::cout << "[INFO] AWindow__isBlockedByLevel 0x" << std::hex << GOffsets.AWindow__isBlockedByLevelOffset << std::dec << std::endl;
        std::cout << "[INFO] ATotem__totemState 0x" << std::hex << GOffsets.ATotem__totemStateOffset << std::dec << std::endl;
        std::cout << "[INFO] AGenerator_activated 0x" << std::hex << GOffsets.AGenerator__activatedOffset << std::dec << std::endl;
        std::cout << "[INFO] AGenerator_generatorChargeable 0x" << std::hex << GOffsets.AGenerator__generatorChargeableOffset << std::dec << std::endl;
        std::cout << "[INFO] AHatch_hatchState 0x" << std::hex << GOffsets.AHatch__hatchStateOffset << std::dec << std::endl;
        std::cout << "[INFO] ABaseTrap_isTrapSet 0x" << std::hex << GOffsets.ABaseTrap__isTrapSetOffset << std::dec << std::endl;
        std::cout << "[INFO] ALocker_playerInLocker 0x" << std::hex << GOffsets.ALocker__playerInLockerOffset << std::dec << std::endl;
        std::cout << "[INFO] BreakableBase_state 0x" << std::hex << GOffsets.BreakableBase__stateOffset << std::dec << std::endl;
        std::cout << "[INFO] DBDAuraComponent_Color 0x" << std::hex << GOffsets.DBDAuraComponent_ColorOffset << std::dec << std::endl;
        std::cout << "[INFO] DBDAuraComponent_MinimumDistanceWhenIsAlwaysVisible 0x" << std::hex << GOffsets.DBDAuraComponent_MinimumDistanceWhenIsAlwaysVisibleOffset << std::dec << std::endl;
        std::cout << "[INFO] DBDAuraComponent_MinimumDistance 0x" << std::hex << GOffsets.DBDAuraComponent_MinimumDistanceOffset << std::dec << std::endl;
        std::cout << "[INFO] DBDGameState_gameLevelEnded 0x" << std::hex << GOffsets.DBDGameState__gameLevelEndedOffset << std::dec << std::endl;
        std::cout << "[INFO] DBDGameState_gameTimedOut 0x" << std::hex << GOffsets.DBDGameState__gameTimedOut << std::dec << std::endl;
        std::cout << "[INFO] DBDGameState_gameLevelCreated 0x" << std::hex << GOffsets.DBDGameState__gameLevelCreated << std::dec << std::endl;
        std::cout << "[INFO] EscapeDoor_activated 0x" << std::hex << GOffsets.EscapeDoor__activatedOffset << std::dec << std::endl;
        std::cout << "[INFO] UStruct_SuperStruct 0x" << std::hex << GOffsets.UStruct_SuperStructOffset << std::dec << std::endl;
        std::cout << "[INFO] RedStainComponent__initialSpotLightColor 0x" << std::hex << GOffsets.RedStainComponent__initialSpotLightColorOffset << std::dec << std::endl;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception loading offsets: " << e.what() << std::endl;
        return false;
    }
}
bool ResolveSignatures() {
    std::cout << "[*] Resolving signatures..." << std::endl;

    // Use a flag to track if we actually need to scan
    bool needsScan = (GOffsets.GWorldOffset == 0 || GOffsets.GObjectsOffset == 0 || GOffsets.GNameOffset == 0);
    if (!needsScan) {
        std::cout << "[*] All core offsets already loaded from file." << std::endl;
        return true;
    }

    // GWorld signature: 48 8B 05 ? ? ? ? 4D 8B C2
    if (GOffsets.GWorldOffset == 0) {
        uintptr_t gWorldAddr = Memory::FindPattern("DeadByDaylight-Win64-Shipping.exe", "48 8B 05 ? ? ? ? 4D 8B C2 4C 8B ? 48 85 D2");
        if (!gWorldAddr) // fallback to shorter
            gWorldAddr = Memory::FindPattern("DeadByDaylight-Win64-Shipping.exe", "48 8B 05 ? ? ? ? 4D 8B C2");
            
        if (gWorldAddr) {
            int32_t relOffset = Memory::Read<int32_t>(gWorldAddr + 3);
            GOffsets.GWorldOffset = (gWorldAddr + 7 + relOffset) - Memory::GetBaseAddress();
            std::cout << "[+] Resolved GWorldOffset: 0x" << std::hex << GOffsets.GWorldOffset << std::dec << std::endl;
        } else {
            std::cerr << "[!] Failed to resolve GWorld signature" << std::endl;
        }
    }

    // GObjects signature: 48 8B 05 ? ? ? ? 48 8B 0C C8
    if (GOffsets.GObjectsOffset == 0) {
        uintptr_t gObjectsAddr = Memory::FindPattern("DeadByDaylight-Win64-Shipping.exe", "48 8B 05 ? ? ? ? 48 8B 0C C8 48 8B 04 D1");
        if (!gObjectsAddr) // fallback to shorter
            gObjectsAddr = Memory::FindPattern("DeadByDaylight-Win64-Shipping.exe", "48 8B 05 ? ? ? ? 48 8B 0C C8");
            
        if (gObjectsAddr) {
            int32_t relOffset = Memory::Read<int32_t>(gObjectsAddr + 3);
            GOffsets.GObjectsOffset = (gObjectsAddr + 7 + relOffset) - Memory::GetBaseAddress();
            std::cout << "[+] Resolved GObjectsOffset: 0x" << std::hex << GOffsets.GObjectsOffset << std::dec << std::endl;
        } else {
            std::cerr << "[!] Failed to resolve GObjects signature" << std::endl;
        }
    }

    // GNames signature: 48 8D 35 ? ? ? ? EB 16
    if (GOffsets.GNameOffset == 0) {
        uintptr_t gNamesAddr = Memory::FindPattern("DeadByDaylight-Win64-Shipping.exe", "48 8D 35 ? ? ? ? EB 16");
        if (gNamesAddr) {
            int32_t relOffset = Memory::Read<int32_t>(gNamesAddr + 3);
            GOffsets.GNameOffset = (gNamesAddr + 7 + relOffset) - Memory::GetBaseAddress();
            std::cout << "[+] Resolved GNameOffset: 0x" << std::hex << GOffsets.GNameOffset << std::dec << std::endl;
        } else {
            std::cerr << "[!] Failed to resolve GNames signature" << std::endl;
        }
    }

    return GOffsets.GWorldOffset != 0 && GOffsets.GObjectsOffset != 0 && GOffsets.GNameOffset != 0;
}
