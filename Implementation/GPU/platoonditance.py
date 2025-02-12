# -*- coding: utf-8 -*-
"""PlatoonDitance.ipynb

Automatically generated by Colab.

Original file is located at
    https://colab.research.google.com/drive/1EbXWRjHeAkUPZ37iTVIMso4yMdRKT-8Q
"""

!apt-get update
!apt-get install -y build-essential
!apt-get install -y cuda

# Commented out IPython magic to ensure Python compatibility.
# %%writefile distance_check.cu
# #include <iostream>
# #include <cuda_runtime.h>
# 
# // CUDA kernel to check distances
# __global__ void checkDistances(float *distanceMatrix, bool *violations, int n, float threshold) {
#     int row = blockIdx.y * blockDim.y + threadIdx.y;
#     int col = blockIdx.x * blockDim.x + threadIdx.x;
# 
#     if (row < n && col < n) {
#         if (row == col) {
#             violations[row * n + col] = false;  // Ignore diagonal
#         } else {
#             violations[row * n + col] = (distanceMatrix[row * n + col] < threshold);
#         }
#     }
# }
# 
# int main() {
#     const int n = 5; // Number of trucks
#     const float threshold = 20.0f; // Threshold in meters
#     float hostMatrix[n * n] = {
#         0, 30, 50, 25, 10,
#         30, 0, 45, 60, 15,
#         50, 45, 0, 10, 20,
#         25, 60, 10, 0, 5,
#         10, 15, 20, 5, 0
#     };
#     bool hostViolations[n * n];
# 
#     // Allocate device memory
#     float *devMatrix;
#     bool *devViolations;
#     cudaMalloc((void**)&devMatrix, n * n * sizeof(float));
#     cudaMalloc((void**)&devViolations, n * n * sizeof(bool));
# 
#     // Copy data to device
#     cudaMemcpy(devMatrix, hostMatrix, n * n * sizeof(float), cudaMemcpyHostToDevice);
# 
#     // Define block and grid dimensions
#     dim3 blockDim(16, 16);
#     dim3 gridDim((n + blockDim.x - 1) / blockDim.x, (n + blockDim.y - 1) / blockDim.y);
# 
#     // Launch kernel
#     checkDistances<<<gridDim, blockDim>>>(devMatrix, devViolations, n, threshold);
# 
#     // Copy results back to host
#     cudaMemcpy(hostViolations, devViolations, n * n * sizeof(bool), cudaMemcpyDeviceToHost);
# 
#     // Print results
#     std::cout << "Distance Matrix (in meters):\n";
#     for (int i = 0; i < n; ++i) {
#         for (int j = 0; j < n; ++j) {
#             std::cout << hostMatrix[i * n + j] << " ";
#         }
#         std::cout << "\n";
#     }
# 
#     std::cout << "\nViolations Matrix:\n";
#     for (int i = 0; i < n; ++i) {
#         for (int j = 0; j < n; ++j) {
#             std::cout << (hostViolations[i * n + j] ? "1" : "0") << " ";
#         }
#         std::cout << "\n";
#     }
# 
#     // Free device memory
#     cudaFree(devMatrix);
#     cudaFree(devViolations);
# 
#     return 0;
# }
#

!nvcc -o distance_check distance_check.cu

!./distance_check

#include <iostream>
#include <cuda_runtime.h>

// CUDA kernel to check distances
__global__ void checkDistances(float *distanceMatrix, bool *violations, int n, float threshold) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < n && col < n) {
        if (row == col) {
            violations[row * n + col] = false;  // Ignore diagonal
        } else {
            violations[row * n + col] = (distanceMatrix[row * n + col] < threshold);
        }
    }
}

int main() {
    const int n = 5; // Number of trucks
    const float threshold = 20.0f; // Threshold in meters
    float hostMatrix[n * n] = {
        0, 30, 50, 25, 10,
        30, 0, 45, 60, 15,
        50, 45, 0, 10, 20,
        25, 60, 10, 0, 5,
        10, 15, 20, 5, 0
    };
    bool hostViolations[n * n];

    // Allocate device memory
    float *devMatrix;
    bool *devViolations;
    cudaMalloc((void**)&devMatrix, n * n * sizeof(float));
    cudaMalloc((void**)&devViolations, n * n * sizeof(bool));

    // Copy data to device
    cudaMemcpy(devMatrix, hostMatrix, n * n * sizeof(float), cudaMemcpyHostToDevice);

    // Define block and grid dimensions
    dim3 blockDim(16, 16);
    dim3 gridDim((n + blockDim.x - 1) / blockDim.x, (n + blockDim.y - 1) / blockDim.y);

    // Create CUDA events
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    // Record start time
    cudaEventRecord(start, 0);

    // Launch kernel
    checkDistances<<<gridDim, blockDim>>>(devMatrix, devViolations, n, threshold);

    // Record stop time
    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);

    // Calculate elapsed time
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    // Copy results back to host
    cudaMemcpy(hostViolations, devViolations, n * n * sizeof(bool), cudaMemcpyDeviceToHost);

    // Print results
    std::cout << "Distance Matrix (in meters):\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << hostMatrix[i * n + j] << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\nViolations Matrix:\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::cout << (hostViolations[i * n + j] ? "1" : "0") << " ";
        }
        std::cout << "\n";
    }

    // Print elapsed time
    std::cout << "\nKernel Execution Time: " << milliseconds << " ms\n";

    // Free device memory
    cudaFree(devMatrix);
    cudaFree(devViolations);

    // Destroy CUDA events
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    return 0;
}

# Commented out IPython magic to ensure Python compatibility.
# %%writefile distance_check.cu
# #include <iostream>
# #include <cuda_runtime.h>
# 
# // CUDA kernel to check distances
# __global__ void checkDistances(float *distanceMatrix, bool *violations, int n, float threshold) {
#     int row = blockIdx.y * blockDim.y + threadIdx.y;
#     int col = blockIdx.x * blockDim.x + threadIdx.x;
# 
#     if (row < n && col < n) {
#         if (row == col) {
#             violations[row * n + col] = false;  // Ignore diagonal
#         } else {
#             violations[row * n + col] = (distanceMatrix[row * n + col] < threshold);
#         }
#     }
# }
# 
# int main() {
#     const int n = 5; // Number of trucks
#     const float threshold = 20.0f; // Threshold in meters
#     float hostMatrix[n * n] = {
#         0, 30, 50, 25, 10,
#         30, 0, 45, 60, 15,
#         50, 45, 0, 10, 20,
#         25, 60, 10, 0, 5,
#         10, 15, 20, 5, 0
#     };
#     bool hostViolations[n * n];
# 
#     // Allocate device memory
#     float *devMatrix;
#     bool *devViolations;
#     cudaMalloc((void**)&devMatrix, n * n * sizeof(float));
#     cudaMalloc((void**)&devViolations, n * n * sizeof(bool));
# 
#     // Copy data to device
#     cudaMemcpy(devMatrix, hostMatrix, n * n * sizeof(float), cudaMemcpyHostToDevice);
# 
#     // Define block and grid dimensions
#     dim3 blockDim(16, 16);
#     dim3 gridDim((n + blockDim.x - 1) / blockDim.x, (n + blockDim.y - 1) / blockDim.y);
# 
#     // Create CUDA events
#     cudaEvent_t start, stop;
#     cudaEventCreate(&start);
#     cudaEventCreate(&stop);
# 
#     // Record start time
#     cudaEventRecord(start, 0);
# 
#     // Launch kernel
#     checkDistances<<<gridDim, blockDim>>>(devMatrix, devViolations, n, threshold);
# 
#     // Record stop time
#     cudaEventRecord(stop, 0);
#     cudaEventSynchronize(stop);
# 
#     // Calculate elapsed time
#     float milliseconds = 0;
#     cudaEventElapsedTime(&milliseconds, start, stop);
# 
#     // Copy results back to host
#     cudaMemcpy(hostViolations, devViolations, n * n * sizeof(bool), cudaMemcpyDeviceToHost);
# 
#     // Print results
#     std::cout << "Distance Matrix (in meters):\n";
#     for (int i = 0; i < n; ++i) {
#         for (int j = 0; j < n; ++j) {
#             std::cout << hostMatrix[i * n + j] << " ";
#         }
#         std::cout << "\n";
#     }
# 
#     std::cout << "\nViolations Matrix:\n";
#     for (int i = 0; i < n; ++i) {
#         for (int j = 0; j < n; ++j) {
#             std::cout << (hostViolations[i * n + j] ? "1" : "0") << " ";
#         }
#         std::cout << "\n";
#     }
# 
#     // Print elapsed time
#     std::cout << "\nKernel Execution Time: " << milliseconds << " ms\n";
# 
#     // Free device memory
#     cudaFree(devMatrix);
#     cudaFree(devViolations);
# 
#     // Destroy CUDA events
#     cudaEventDestroy(start);
#     cudaEventDestroy(stop);
# 
#     return 0;
# }
#

!nvcc -o distance_check distance_check.cu

!./distance_check