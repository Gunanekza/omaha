// Copyright 2019 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OMAHA_GOOPDATE_DM_STORAGE_H__
#define OMAHA_GOOPDATE_DM_STORAGE_H__

#include <windows.h>
#include <atlpath.h>
#include <atlstr.h>

#include "base/basictypes.h"
#include "omaha/base/constants.h"
#include "omaha/goopdate/dm_messages.h"

namespace omaha {

// This is the standard name for the file that PersistPolicies() uses for each
// {policy_type} that it receives from the DMServer.
const TCHAR kPolicyResponseFileName[] = _T("PolicyFetchResponse");

// A handler for storage related to cloud-based device management of Omaha. This
// class provides access to an enrollment token, a device management token, and
// a device identifier.
class DmStorage {
 public:
  // The possible sources of an enrollment token, sorted by decreasing
  // precedence.
  enum EnrollmentTokenSource {
    kETokenSourceNone,
    kETokenSourceCompanyPolicy,
#if defined(HAS_LEGACY_DM_CLIENT)
    kETokenSourceLegacyPolicy,
    kETokenSourceOldLegacyPolicy,
#endif  // defined(HAS_LEGACY_DM_CLIENT)
    kETokenSourceRuntime,
    kETokenSourceInstall,
  };

  static HRESULT CreateInstance(const CString& enrollment_token);
  static void DeleteInstance();

  static DmStorage* Instance();

  // Returns the current enrollment token, reading from sources as-needed to
  // find one. Returns an empty string if no enrollment token is found.
  CString GetEnrollmentToken();

  // Returns the origin of the current enrollment token, or kETokenSourceNone if
  // none has been found.
  EnrollmentTokenSource enrollment_token_source() const {
    return enrollment_token_source_;
  }

  // Writes the instance's enrollment token if it was provided at runtime into
  // Omaha's ClientState key so that it is available for subsequent runs.
  // Returns S_FALSE if the instance's enrollment token was not provided at
  // runtime.
  HRESULT StoreRuntimeEnrollmentTokenForInstall();

  // Returns the device management token, reading from sources as-needed to find
  // one. Returns an empty string if no device management token is found.
  CStringA GetDmToken();

  // Writes |dm_token| into the registry.
  HRESULT StoreDmToken(const CStringA& dm_token);

  // Returns the device identifier, or an empty string in case of error.
  CString GetDeviceId();

  // Persists each PolicyFetchResponse in |responses| into a subdirectory within
  // |policy_responses_dir|. Each PolicyFetchResponse is stored within a
  // subdirectory named {Base64Encoded{policy_type}}, with a fixed file name of
  // "PolicyFetchResponse", where the file contents are
  // {SerializeToString-PolicyFetchResponse}}.
  //
  // Each PolicyFetchResponse file is opened in exclusive mode. If we are unable
  // to open or write to this file, the caller is expected to try again later.
  // For instance, if UA is calling us, UA will retry at the next UA interval.
  //
  // Client applications could use ::FindFirstChangeNotificationW on the
  // subdirectory corresponding to their respective policy_type to watch for
  // changes. They can then read and apply the policies within this file.
  // To minimize the number of notifications for existing PolicyFetchResponse
  // files, the files are first modified in-place if the response includes them,
  // and then the files that do not have a corresponding response are deleted.
  static HRESULT PersistPolicies(const CPath& policy_responses_dir,
                                 const PolicyResponsesMap& responses);

 private:
  // Constructs an instance with a runtime-provided enrollment token (e.g., one
  // obtained via the etoken extra arg).
  explicit DmStorage(const CString& runtime_enrollment_token);

  // The possible sources of a device management token, sorted by decreasing
  // precedence.
  enum DmTokenSource {
    kDmTokenSourceNone,
    kDmTokenSourceCompany,
#if defined(HAS_LEGACY_DM_CLIENT)
    kDmTokenSourceLegacy,
#endif  // defined(HAS_LEGACY_DM_CLIENT)
  };

  void LoadEnrollmentTokenFromStorage();
  void LoadDmTokenFromStorage();
  void LoadDeviceIdFromStorage();

  // An enrollment token provided on the command line at runtime.
  const CString runtime_enrollment_token_;

  // The active enrollment token.
  CString enrollment_token_;

  // The active device management token.
  CStringA dm_token_;

  // The device identifier.
  CString device_id_;

  // The origin of the current enrollment token.
  EnrollmentTokenSource enrollment_token_source_;

  // The origin of the current device management token.
  DmTokenSource dm_token_source_;

  static DmStorage* instance_;

  friend class DmStorageTest;

  DISALLOW_COPY_AND_ASSIGN(DmStorage);
};

}  // namespace omaha

#endif  // OMAHA_GOOPDATE_DM_STORAGE_H__
