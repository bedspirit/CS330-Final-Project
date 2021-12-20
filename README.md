# CS330-Final-Project
Cigar
This project called for the recreation of a 2D scene with relatively simple geometry.  The picture of a cigar on an ashtray with a cup on a coaster met the requirement of being re-creatable with simple shapes while also making for an interesting picture.
Four primitive shapes were to be used in the creation of this image.  My image includes these shapes:
•	I used cylinders to make the cup, the cigar, and the ashtray.
•	I used a plane for the surface the objects are placed upon
•	I used a cone at the end of the cigar
•	I used rectangular cubes for the coaster
•	I used cylinders to make the cup, the cigar, and the ashtray.
•	I used a plane for the surface the objects are placed upon
•	I used a cone at the end of the cigar
•	I used rectangular cubes for the coaster
![image](https://user-images.githubusercontent.com/61409173/146706429-b50262a6-5505-4a38-af53-c291d0d87c08.png)
There are many triangles used in the higher poly image that aren’t essential to the shape of the object.  They were used to prevent texture warping.  OpenGL only accepts triangular faces which can be problematic if you end up with some long slender triangles.  Adjusting those triangles can warp their texture very easily because of the small amount of surface area between edges.  The original version was designed to prevent that from happening.  The lower poly one does less to address that, but since it’s not going to be re-used, I considered it safe.
We were required to use a royalty free image of 1024x1024 or higher.  My texture is 6251x2150.  I wanted to create the illusion that all the objects were occupying the same space so I edited my textures to give them shadows and ambient occlusion.  
I used two lights using the phong shading model.  I set the main light above the model and set the ambient and diffuse values very high but I set the specularity low.  I didn’t want a large hard shine from the light, but I did want a soft, subtle shine, to imply the lights presence.  I used a small red point light near the tip of the cigar.  I set the specularity pretty high on that one but the ambience and diffuse was set low.  I didn’t want that light to color the whole image, I just wanted the area near the tip of the cigar.  The specularity was set high because I wanted a bright reflection from that light.  That might not be realistic, but it creates a more appealing image.  I should also point out that I employed a small cheat on my texture that put a red glow right where the cherry of the cigar sits.  This look can be achieved in OpenGL using their Bloom lighting technique, but it involves combining two separate renders together and was well beyond the scope of this course.
Navigation in the executable is as follows:
•	WASD to move forward, left, right, and back
•	Q and E to move up and down
•	Mouse to look around with the Camera
•	Scroll the mouse wheel to adjust speed
•	Spacebar and P to change the view from orthographic to perspective.

The part of this code that is most reusable is the camera.h file.  It can be inserted into any solution and used.  I can attest to the reusability of the rest of the code because I reused for every assignment in the course.  There are modular functions that I copy and paste.
