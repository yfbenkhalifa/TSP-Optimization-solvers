# Use a more complete base image
FROM python:3.9

# Install dependencies that CPLEX might need
RUN apt-get update && apt-get install -y \
    libgomp1 \
    libc6 \
    libstdc++6 \
    libx11-6 \
    libxext6 \
    libxrender1 \
    libxtst6 \
    libgtk2.0-0 \
    libxt6 \
    libxmu6 \
    xauth \
    xvfb \
    && rm -rf /var/lib/apt/lists/*

# Copy the CPLEX binary into the container
COPY cplex_installer.bin /tmp/

# Run the installer with verbose output and logging
RUN chmod +x /tmp/cplex_installer.bin && \
    Xvfb :1 -screen 0 1024x768x16 & export DISPLAY=:1 && \
    /tmp/cplex_installer.bin -f --verbose | tee /tmp/install.log

# Set up environment variables for CPLEX
ENV CPLEX_HOME /opt/ibm/ILOG/CPLEX_Studio
ENV PATH="${CPLEX_HOME}/cplex/bin/x86-64_linux:${PATH}"
ENV LD_LIBRARY_PATH="${CPLEX_HOME}/cplex/lib/x86-64_linux:${LD_LIBRARY_PATH}"
ENV PYTHONPATH="${CPLEX_HOME}/cplex/python/3.9/x86-64_linux:${PYTHONPATH}"

# Verify installation
RUN python3 -c "import cplex; print(cplex.__version__)"
