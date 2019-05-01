FROM ubuntu:16.04

RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:jonathonf/python-3.6 && \
    add-apt-repository ppa:git-core/ppa && \
    apt-get update && \
    apt-get install -y python3.6 curl build-essential cmake libxml2-dev zlib1g-dev git && \
    curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.deb.sh | bash && \
    apt-get install -y git-lfs && \
    git lfs install && \
    curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py && python3 get-pip.py && \
    pip install conan && \
    conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

COPY . /fbx2gltf

WORKDIR /fbx2gltf

# Pull the fbx sdk
RUN git lfs pull

# Build and install
RUN conan install . -i build -s build_type=Release -e FBXSDK_SDKS=/fbx2gltf/sdk && \
    conan build -bf build . && \
    cp build/FBX2glTF /usr/bin && \
    cd / && \
    rm -rf /fbx2gltf
