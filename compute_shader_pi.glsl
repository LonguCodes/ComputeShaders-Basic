#version 430
#extension GL_ARB_compute_shader :                  enable
#extension GL_ARB_shader_storage_buffer_object :     enable
layout (std430, binding=1) buffer Data{
    double data[];
};

layout( local_size_x = 1024,  local_size_y = 1, local_size_z = 1 )   in;

void main()
{
    uint gid = gl_GlobalInvocationID.x;
    bool isEven = gid%2 ==1;
    data[gid] = pow(1,gid)/(2.0LF*gid+1); 
    if(isEven)
        data[gid]= -data[gid];
}