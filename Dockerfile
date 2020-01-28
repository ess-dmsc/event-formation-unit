FROM screamingudder/ubuntu18.04-build-node:3.0.0

ENV DEBIAN_FRONTEND=noninteractive

ARG http_proxy

ARG https_proxy

ARG local_conan_server

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
COPY src efu_src/src
COPY CMakeLists.txt LICENSE README.md Utilities.cmake efu_src/

RUN cd efu && \
    cmake -DCONAN="MANUAL" ../efu_src && \
    make -j4

COPY docker_launch.sh /
CMD ["./docker_launch.sh"]
