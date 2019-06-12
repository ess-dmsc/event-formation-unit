FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

ARG http_proxy

ARG https_proxy

ARG local_conan_server

RUN apt-get update -y && \
    apt-get --no-install-recommends -y install build-essential git python-pip cmake tzdata vim m4 flex && \
    apt-get -y autoremove && \
    apt-get clean all && \
    rm -rf /var/lib/apt/lists/*

RUN pip install --upgrade pip==9.0.3 && pip install setuptools && \
    pip install conan && \
    rm -rf /root/.cache/pip/*

# Force conan to create .conan directory and profile
RUN conan profile new default

# Replace the default profile and remotes with the ones from our Ubuntu build node
RUN conan config install http://github.com/ess-dmsc/conan-configuration.git
ADD "https://raw.githubusercontent.com/ess-dmsc/docker-ubuntu18.04-build-node/master/files/default_profile" "/root/.conan/profiles/default"

# Add local Conan server if one is defined in the environment
RUN if [ ! -z "$local_conan_server" ]; then conan remote add --insert 0 ess-dmsc-local "$local_conan_server"; fi

# Do the Conan install step first so that we don't have to rebuild all dependencies if something changes in the efu source
RUN mkdir efu
RUN mkdir efu_src
COPY conanfile.txt efu_src/
RUN cd efu && conan install --build=outdated ../efu_src/conanfile.txt

COPY cmake efu_src/cmake
COPY utils/udp efu_src/utils/udp
COPY utils/udpredirect efu_src/utils/udpredirect
COPY prototype2 efu_src/prototype2
COPY CMakeLists.txt LICENSE README.md Utilities.cmake efu_src/

RUN cd efu && \
    cmake -DCONAN="MANUAL" ../efu_src && \
    make -j4

COPY docker_launch.sh /
CMD ["./docker_launch.sh"]
