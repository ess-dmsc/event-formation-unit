ARG VARIANT="ubuntu-22.04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# This Dockerfile's base image has a non-root user with sudo access. Use the "remoteUser"
# property in devcontainer.json to use it. On Linux, the container user's GID/UIDs
# will be updated to match your local UID/GID (when using the dockerFile property).
# See https://aka.ms/vscode-remote/containers/non-root-user for details.
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID


# Configure apt and install packages
RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    #
    # Install
    && apt-get -y install \
    build-essential \
    gdb \
    cmake \
    cppcheck \
    doxygen \
    graphviz \
    libsasl2-dev \
    libssl-dev \
    git \
    zip \
    unzip \
    vim \
    # Python
    python3-pip \
    python-is-python3 \
    python3.10-venv \
    # libpcap
    tcpdump \
    # coverage
    lcov \
    gcovr \
    #
    # [Optional] Update UID/GID if needed
    && if [ "$USER_GID" != "1000" ] || [ "$USER_UID" != "1000" ]; then \
    groupmod --gid $USER_GID $USERNAME \
    && usermod --uid $USER_UID --gid $USER_GID $USERNAME \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME; \
    fi \
    #
    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Install clangd-16 stable formater
RUN printf "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main\ndeb-src http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" > /etc/apt/sources.list.d/clangd.list \
    && wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install \
    clangd-16 \
    clang-format-16 \
    && ln -s /usr/bin/clangd-16 /usr/bin/clangd

# Install dependency manager Conan
RUN pip3 install "conan<2"
# RUN pip3 install conan \
#     && conan profile new default --detect \
#     && conan profile update settings.compiler.libcxx=libstdc++11 default
