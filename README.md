# vulkanApplication
A basic vulkan application.
>>This is a demo that draws a triangle, implemented using vulkan.
>>The application is based on the windows platform and developed using vs2015.
>>The program will draw a triangle on the screen and continue to upload.

		vertex shader:
		#version 400
		#extension GL_ARB_separate_shader_objects : enable//启动GL_ARB_separate_shader_objects
		#extension GL_ARB_shading_language_420pack : enable//启动GL_ARB_shading_language_420pack
		layout (std140,set = 0, binding = 0) uniform bufferVals {//一致块
    			mat4 mvp;//总变换矩阵
		} myBufferVals;
		layout (location = 0) in vec3 pos;//传入的物体坐标系顶点位置
		layout (location = 1) in vec3 color;//传入的顶点颜色
		layout (location = 0) out vec3 vcolor;//传到片元着色器的顶点颜色
		out gl_PerVertex {//输出的接口块
			vec4 gl_Position;//顶点最终位置
		};
		void main() {//主函数
    		gl_Position = myBufferVals.mvp * vec4(pos,1.0);//计算最终顶点位置
			vcolor=color;//传递顶点颜色给片元着色器
		}

		fragment shader:
		#version 400
		#extension GL_ARB_separate_shader_objects : enable//启动GL_ARB_separate_shader_objects
		#extension GL_ARB_shading_language_420pack : enable//启动GL_ARB_shading_language_420pack
		layout (location = 0) in vec3 vcolor;//顶点着色器传入的顶点颜色数据
		layout (location = 0) out vec4 outColor;//输出到渲染管线的片元颜色值
		void main() {
   			outColor=vec4(vcolor.rgb,1.0);//将顶点着色器传递过来的颜色值输出
		}
