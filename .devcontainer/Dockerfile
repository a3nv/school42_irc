FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Base tools & compilers (42-like)
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    g++ \
    make \
    cmake \
    gdb \
    lldb \
    valgrind \
    git \
    curl \
    wget \
    zsh \
    vim \
    neovim \
    netcat-openbsd \
    python3 \
    python3-pip \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Optional: Norminette for other 42 projects
RUN pip3 install --no-cache-dir norminette

# Workdir where VS Code will mount the repo by default
WORKDIR /workspaces/42prague_webserv
