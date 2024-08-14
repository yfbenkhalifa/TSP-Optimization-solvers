# Use the latest Ubuntu image as the base
FROM ubuntu:latest

# Set the label for the authors
LABEL authors="yfben"

# Install necessary dependencies
RUN apt-get update && apt-get install -y \
    unzip \
    tar \
    && rm -rf /var/lib/apt/lists/*

# Copy the CPLEX binary file into the Docker image
# Assuming the binary file is named cplex_installer.bin and is in the same directory as the Dockerfile
COPY cplex_installer.bin /tmp/cplex_installer.bin

# Make the binary file executable
RUN chmod +x /tmp/cplex_installer.bin

# Run the CPLEX installer
RUN /tmp/cplex_installer.bin -f /tmp/installer.properties

# Set environment variables if needed
# ENV CPLEX_HOME /path/to/cplex
# ENV PATH $CPLEX_HOME/bin:$PATH
# ENV LD_LIBRARY_PATH $CPLEX_HOME/lib:$LD_LIBRARY_PATH

# Define the entry point or command for the container
ENTRYPOINT ["top", "-b"]