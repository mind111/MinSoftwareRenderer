#  SoftwareRenderer
* This project is still a work in progress until I am satisfied with the work that I put in :)  

## Progress
* Finished implementing Cook-Torrance specular BRDF and tested using debug_scene. 

* Current implementation of SSAO computes occlusion in one direction only using difference in depth. Thus result is not as apparent as I expected in the final render. Here is a comparison. Flat shader is used to allow more obvious visual difference.   
![no-ssao-iamge](images/ssao/no-ssao.png)


### TODO
* Add few other scenes, and tweak the posing of models used in the scene
* Maybe camera control?? or maybe give a simple gui that allow me to easily adjust camera 
* SIMD for performance improvements, currently in debug build, drawing one frame of default_scene can goes up to 160ms with SSAO turned on. 

* Multi-threading

## Overview

##  Study Notes

######  This documentation is meant as study notes that compiled from my experiences of implementing this project, so that I can keep things more organized and make it a more meaningful learning experience

* This personal project is inspired by following two amazing works https://github.com/ssloy/tinyrenderer/wiki and https://github.com/Angelo1211/SoftwareRenderer. It serves the purpose for me to get into nitty-gritty details of computer graphics out of my strong interest in the discipline.

###### Triangle Rasterization

* Given three vertices of a triangle, define a bounding box to minimize number of pixels that we need to scan through

* Then traverse every pixels within the bounding box

  * ```pseudocode
    For each pixel within BoundingBox：
    	Compute pixel's barycentric coordinates
    	Determine if the pixel overlaps the triangle:
    		pixel shading
    ```
  * For computing the barycentric coordinates, I adopted Cramer's rule to solve the linear system

###### Wavefront .obj file
* Models used for all the scenes are mostly in .obj format because of it's simplicity. I wrote a simple .obj loader that is far from optimal, but it serves the purpose. 

###### Matrix Transformation pipeline

* Model -> World: Model matrix is pretty intuitive. It can be further decomposed in to 
  * Translation * Rotation *  Scale (order matters!!!)

* World -> Camera:
  * View transformation: by default, the camera is looking down -z direction, in this case, after the view transformation, all the vertices 

* Camera -> Clip:
    * I misunderstood the camera's forward vector. I used to thought forward should pointing to camera's target, while the opposite is true. Though camera's looking down -z direction, its initial three axis should still be aligned with directions of world space axises. In this case, the forward vector should be derived using ```camera.position - target.position```. 

###### Normal Mapping