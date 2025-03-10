# 数字智能识别系统

<div style="margin-left:50px; font-size: 30px;">CMAKE Valid Variants</div>

| OPTIONS          | TYPE   | DEFAULT | DESCRIPTION|
| ----             | ----   | ----  | ---                                        |
| CMAKE_BUILD_TYPE | STRING | DEBUG | BUILD TYPE                                 |
| ENABLE_GPU       | BOOL   | ON    | ENABLE CUDA(ONLY SUPPORT CUDA)             |
| ENABLE_MP        | BOOL   | ON    | ENABLE Multiple Process                    | 
| ENABLE_EIGEN     | BOOL   | ON    | ENABLE Eigen To Accelerate Mat Calucalting |



`CMake` Configure & Build
```CMake
mkdir build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# Ninja Generator 
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..

# Build
cmake --build .

# Run
./bin/inf_qwq
```





### 有问题请提 **Issue** !!!
