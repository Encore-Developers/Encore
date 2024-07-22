#pragma once

// The numeric version format is AAAAABBBBBCCCCCDDDE where:
//
// AAAAA - major version number
// BBBBB - minor version number
// CCCCC - bugfix version number
// DDD   - alpha / beta (DDD + 500) version number
// E     - final (0) / snapshot (1)
//
// When DDDE is not 0, 1 is subtracted from AAAAABBBBBCCCCC. For example:
//
// Version      AAAAABBBBBCCCCCDDDE
//
// 0.1.0        0000000001000000000
// 0.1.2        0000000001000020000
// 1.2.3        0000100002000030000
// 2.2.0-a.1    0000200001999990010
// 3.0.0-b.2    0000299999999995020
// 2.2.0-a.1.z  0000200001999990011
//
#define LIBSTUD_UUID_VERSION       $libstud_uuid.version.project_number$ULL
#define LIBSTUD_UUID_VERSION_STR   "$libstud_uuid.version.project$"
#define LIBSTUD_UUID_VERSION_ID    "$libstud_uuid.version.project_id$"

#define LIBSTUD_UUID_VERSION_MAJOR $libstud_uuid.version.major$
#define LIBSTUD_UUID_VERSION_MINOR $libstud_uuid.version.minor$
#define LIBSTUD_UUID_VERSION_PATCH $libstud_uuid.version.patch$

#define LIBSTUD_UUID_PRE_RELEASE   $libstud_uuid.version.pre_release$

#define LIBSTUD_UUID_SNAPSHOT_SN   $libstud_uuid.version.snapshot_sn$ULL
#define LIBSTUD_UUID_SNAPSHOT_ID   "$libstud_uuid.version.snapshot_id$"
