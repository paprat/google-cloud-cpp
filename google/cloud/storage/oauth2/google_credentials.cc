// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/storage/oauth2/google_credentials.h"
#include "google/cloud/internal/throw_delegate.h"
#include "google/cloud/storage/internal/nljson.h"
#include "google/cloud/storage/oauth2/anonymous_credentials.h"
#include "google/cloud/storage/oauth2/authorized_user_credentials.h"
#include "google/cloud/storage/oauth2/google_application_default_credentials_file.h"
#include "google/cloud/storage/oauth2/service_account_credentials.h"
#include <fstream>
#include <iterator>

namespace google {
namespace cloud {
namespace storage {
inline namespace STORAGE_CLIENT_NS {
namespace oauth2 {

std::shared_ptr<Credentials> GoogleDefaultCredentials() {
  auto path = GoogleAdcFilePathOrEmpty();
  if (not path.empty()) {
    std::ifstream is(path);
    if (not is.is_open()) {
      google::cloud::internal::RaiseRuntimeError(
          "Cannot open credentials file " + path);
    }
    std::string contents(std::istreambuf_iterator<char>{is}, {});
    auto cred_json =
        storage::internal::nl::json::parse(contents, nullptr, false);
    if (cred_json.is_discarded()) {
      google::cloud::internal::RaiseRuntimeError(
          "Invalid contents in credentials file " + path);
    }
    std::string cred_type = cred_json.value("type", "no type given");
    if (cred_type == "authorized_user") {
      return std::make_shared<AuthorizedUserCredentials<>>(contents, path);
    }
    if (cred_type == "service_account") {
      return std::make_shared<ServiceAccountCredentials<>>(contents, path);
    }
    google::cloud::internal::RaiseRuntimeError(
        "Unsupported credential type (" + cred_type +
        ") when reading Application Default Credentials file from " + path +
        ".");
  }

  // TODO(#579): Check for implicit environment-based credentials if no ADC file
  // was specified.

  google::cloud::internal::RaiseRuntimeError(
      "No eligible credential types were found to use as default credentials.");
}

std::shared_ptr<Credentials> CreateAnonymousCredentials() {
  return std::make_shared<AnonymousCredentials>();
}

std::shared_ptr<Credentials>
CreateAuthorizedUserCredentialsFromJsonFilePath(std::string const& path) {
  std::ifstream is(path);
  std::string contents(std::istreambuf_iterator<char>{is}, {});
  return std::make_shared<AuthorizedUserCredentials<>>(contents, path);
}

std::shared_ptr<Credentials>
CreateAuthorizedUserCredentialsFromJsonContents(std::string const& contents) {
  return std::make_shared<AuthorizedUserCredentials<>>(contents, "memory");
}

std::shared_ptr<Credentials>
CreateServiceAccountCredentialsFromJsonFilePath(std::string const& path) {
  std::ifstream is(path);
  std::string contents(std::istreambuf_iterator<char>{is}, {});
  return std::make_shared<ServiceAccountCredentials<>>(contents, path);
}

std::shared_ptr<Credentials>
CreateServiceAccountCredentialsFromJsonContents(std::string const& contents) {
  return std::make_shared<ServiceAccountCredentials<>>(contents, "memory");
}

}  // namespace oauth2
}  // namespace STORAGE_CLIENT_NS
}  // namespace storage
}  // namespace cloud
}  // namespace google
