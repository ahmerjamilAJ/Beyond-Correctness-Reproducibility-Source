FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential clang-15 llvm-15-dev llvm-15-tools cmake ninja-build \
    git python3 python3-pip python3-venv binutils ca-certificates \
    zlib1g-dev libedit-dev libxml2-dev && rm -rf /var/lib/apt/lists/*
RUN python3 -m pip install --no-cache-dir PyYAML==6.0.3

RUN git clone https://github.com/DependableSystemsLab/LLTFI.git /opt/LLTFI \
    && git -C /opt/LLTFI checkout 66dad0d3314ab72eeb82384fc21066f88615b358
COPY environment/configure_llfi.py /opt/bc-setup/configure_llfi.py
RUN python3 /opt/bc-setup/configure_llfi.py --source /opt/LLTFI --llvm /usr/lib/llvm-15 \
    && cmake -S /opt/LLTFI -B /opt/LLTFI/_install -DNO_GUI=ON \
    && cmake --build /opt/LLTFI/_install -j2 \
    && python3 /opt/bc-setup/configure_llfi.py --source /opt/LLTFI --installed /opt/LLTFI/_install

COPY fi/runtime /opt/bc-artifact/fi/runtime
COPY environment/build_runtime.py /opt/bc-artifact/environment/build_runtime.py
RUN python3 /opt/bc-artifact/environment/build_runtime.py --out /opt/bc-runtime --jobs 1
ENV PATH="/opt/LLTFI/_install/bin:/usr/lib/llvm-15/bin:${PATH}"
ENV LD_LIBRARY_PATH="/opt/bc-runtime:/usr/lib/llvm-15/lib"
ENV BC_LLFI_RUNTIME="/opt/bc-runtime/libllfi-rt.so"
ENV PYTHONDONTWRITEBYTECODE=1
WORKDIR /workspace
