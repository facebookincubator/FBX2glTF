// If this file is named draco_version.cc.cmake:
// This file is used as input at cmake generation time.

// If this file is named draco_version.cc:
// GENERATED FILE, DO NOT EDIT. SEE ABOVE.
#include "draco_version.h"

static const char kDracoGitHash[] = "063994c362871d6f149c24c669122e4ef3fa8196";
static const char kDracoGitDesc[] = "unreleased";

const char *draco_git_hash() {
  return kDracoGitHash;
}

const char *draco_git_version() {
  return kDracoGitDesc;
}

const char* draco_version() {
  return draco::Version();
}
