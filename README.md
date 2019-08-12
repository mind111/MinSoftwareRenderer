#  Graphx

##  Study Notes

######  This documentation is meant as study notes that compiled from my experiences of implementing this project, so that I can keep things more organized and make it a more meaningful learning experience

* This personal project is inspired by this amazing piece of material https://github.com/ssloy/tinyrenderer/wiki. It serves the purpose for me to study computer graphics out of my strong interest in the discipline.

* I try to implement a software renderer from scratch following this material while not fully depending on the provided source code, since I want to struggle through the details to learn. Therefore, my implementation is probably crappy and far from optimal. If I do get stuck for a long time at some point, I would look for some hints from the material and source code.

######  Line Drawing

* My crappy implementation
  * Naively interpret the math of Bresenham's line drawing to C++, which is a lot slower than the version provided in "tinyrenderer"
  * Comparison
    * 
    * 

###### Triangle Rasterization

* Given three vertices of a triangle, define a bounding box to minimize number of pixels that we need to scan through

* Then scan through every pixels within the bounding box

  * ```pseudocode
    For each pixel within BoundingBox：
    	Compute pixel's barycentric coordinates
    	Determine if the Pixel IsInside of the triangle:
    		set pixel color
    ```

  * For computing the barycentric coordinates, I adopted Cramer's rule to solve the linear system

###### Wavefront .obj file

* I wrote a crappy function that read vertex data from a `.obj` file, it only reads vertex positions
* When implementing this `ParseObj()` function, I learned another lesson on pointers
  * **Gotcha**
    * Passing pointers as parameter into a function and allocate memory on the pointer does not work because
      * Inside the function, the pointer is passed in as a copy, so if you allocate memory for the "copy", the original pointer will still point to where it used to point to. Instead, the "copy" will point to the allocated memory
      * In this case, to pass pointers as parameters and correctly allocate memory for them, I had to pass in pointer to those pointers.
    * I ran into another gotcha when using a `int*` to traverse an `int[]`. I was assuming that once the pointer passes the end of the array, it would point to null, and thus was using ``` while (ptr)``` to dictate when traversal should stop without realizing that even if `ptr` went pass the end of the array, it can still point to a non-null memory that is used by something else in the program or whatever and thus trigger an access violation. Be careful with pointer arithmetic!!!  