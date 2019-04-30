FROM ubuntu:16.04

RUN apt-get update && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:jonathonf/python-3.6 && \
    apt-get update && \
    apt-get install -y python3.6 curl build-essential cmake libxml2-dev zlib1g-dev git && \
    curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py && python3 get-pip.py && \
    pip install conan && \
    conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan

# Install FBX SDK
WORKDIR /fbxsdktemp

RUN curl -L https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/20192/fbx20192_fbxsdk_linux.tar.gz -o fbx20192_fbxsdk_linux.tar.gz && \
	tar -xvf fbx20192_fbxsdk_linux.tar.gz && \
	echo "yes\nn" | ./fbx20192_fbxsdk_linux /usr && \
	rm -rf /fbxsdktemp

COPY . /fbx2gltf

WORKDIR /fbx2gltf

RUN conan install . -i build -s build_type=Release -e FBXSDK_SDKS=sdk && \
    conan build -bf build .

