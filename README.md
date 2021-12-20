# CS330-Final-Project
Cigar
![image](https://user-images.githubusercontent.com/61409173/146706479-85d56e7e-c953-48d8-b6bb-7600e8f5ba75.png)
![image](https://user-images.githubusercontent.com/61409173/146706491-07f3bb1e-5507-42f7-9fa1-bf3b60058fa0.png)


How do I approach designing software?
What new design skills has your work on the project helped you to craft?

It gave me experience with OpenGL.  I relied on my preexisting design skills to try to get the best results from an assignmnet which limited what kinds of models and methods could be employed.

What design process did you follow for your project work?

I put a lot of energy preparing the scene outside of OpenGL.  For the OpenGL portions, I just made sure the lights, camera, perspective, and backgournd colors were working properly.

How could tactics from your design approach be applied in future work?

If I ever use OpenGL, I will likely use a game engine to generate some of the pedestrian code.  Unless you're developing your own engine, I don't see the need to type all of this code out.

How do I approach developing programs?

I do an outline and then I test as i go.

What new development strategies did you use while working on your 3D scene?

I spent a ridiculous aboutn of time reading through sample code.

How did iteration factor into your development?

I used iteration in other applicatoins to generate much of the data

How has your approach to developing code evolved throughout the milestones, which led you to the project’s completion?

It hasn't, sadly.  This is the first time I found myself consistently behind.  I always want to implement something that goes above and beyond and sometimes that can waste alot of time.

How can computer science help me in reaching my goals?

I code for a living.  My education focused on math, so having an educational background in computer science will strengthen my resume.

How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?

Imagery is an effective tool in getting people's attention.

How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?

I've worked on some commericial projects,  if any of them had hit, I would have my choice in future projects.

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
