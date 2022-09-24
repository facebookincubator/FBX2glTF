
function kkrun() {
	echo "$*"
	eval "$*"
}

# Determine SDK location & build settings for Linux vs (Recent) Mac OS X
if [[ "$OSTYPE" == "darwin"* ]]; then
    export CONAN_CONFIG="-s compiler=apple-clang -s compiler.version=10.0 -s compiler.libcxx=libc++"
    export FBXSDK_TARBALL="https://github.com/zellski/FBXSDK-Darwin/archive/2019.2.tar.gz"
    export FBXSDK_INCLUDE_DIR="$PWD/../FBX2glTF/sdk/Darwin/2019.2/include"
elif [[ "$OSTYPE" == "linux"* ]]; then
    export CONAN_CONFIG="-s compiler.libcxx=libstdc++11"
    export FBXSDK_TARBALL="https://github.com/zellski/FBXSDK-Linux/archive/2019.2.tar.gz"
    export FBXSDK_INCLUDE_DIR="$PWD/../FBX2glTF/sdk/Darwin/2019.2/include"
else
    echo "This snippet only handles Mac OS X and Linux."
fi

echo "export CONAN_CONFIG=\"$CONAN_CONFIG\""
echo "export FBXSDK_TARBALL=\"$FBXSDK_TARBALL\""
echo "export FBXSDK_INCLUDE_DIR=\"$FBXSDK_INCLUDE_DIR\""

if [ ! -d "sdk" ]; then
	echo "[Fetch and unpack FBX SDK]"
	echo "curl -sL '${FBXSDK_TARBALL}' | tar xz --strip-components=1 --include */sdk/"
	curl -sL "${FBXSDK_TARBALL}" | tar xz --strip-components=1 --include */sdk/
	echo "[Then decompress the contents]"
	kkrun "zstd -d -r --rm sdk"
fi

echo "[Install and configure Conan, if needed]"
kkrun "pip3 install conan" # or sometimes just "pip"; you may need to install Python/PIP
kkrun "conan remote add --force bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan"
kkrun "conan config set general.revisions_enabled=1"

echo "[Initialize & run build]"
kkrun "conan install . -i build -s build_type=Release ${CONAN_CONFIG} --build=missing"
kkrun "conan build . -bf build"
