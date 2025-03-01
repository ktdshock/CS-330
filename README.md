# CS-330 / Computer graphics and visualization

<b>Approaching Software Design</b>

Designing software requires careful planning, iteration, and problem-solving. Throughout this project, I followed a structured approach to build a 3D scene using OpenGL. The first step in my design process was identifying the key objects in my scene, including a lamp, cup, pencils, and a wooden desk. I then organized my code by separating responsibilities into different files, such as SceneManager for rendering and ViewManager for camera control.

A well-thought-out design process is important because it ensures that the final product is not only functional but also maintainable. By structuring my code in a modular way, I was able to make changes easily and debug issues efficiently. The most important lesson I learned from this process is that breaking a project into small, manageable steps leads to better results.

-----------------------------------------------------------------------------

<b>New Design Skills and Development Strategies</b>

Working on this project helped me develop several new design and programming skills. One of the most important was learning how to use materials and lighting effectively. Instead of simply assigning colors to objects, I applied ambient, diffuse, and specular properties to create realistic shading and reflections.

Another major skill I improved was scene composition—understanding how to position objects in 3D space so that they look natural. Initially, some objects were misaligned or rotated incorrectly, but through iteration, I adjusted their positions and transformations to achieve a more polished look.

In terms of development strategies, I learned the importance of debugging early and often. For example, when I first applied textures to my scene, the wooden desk appeared completely black due to a missing texture mapping step. By methodically testing different solutions, I was able to resolve the issue and successfully load textures. This experience reinforced the value of testing small changes frequently to avoid bigger issues later.

-----------------------------------------------------------------------------

<b>Iterative Development and Refinement</b>

One of the key takeaways from this project was the role of iteration in software development. Initially, my lamp’s lighting was too bright, making it look completely yellow. After adjusting the light source intensity and adding a second light with a slightly different color, I was able to make the scene look more natural.

Iteration also played a major role in object positioning and rotation. For example, when I first added the cup and pencils, they appeared tilted at an unnatural angle. By resetting their rotation values and carefully adjusting their Y-axis rotation, I achieved a more realistic placement where both pencils were clearly visible. These small refinements added up to make a big difference in the final quality of the scene.



<b>How My Coding Approach Evolved</b>

At the beginning of the project, my approach to coding was focused on simply getting the objects to appear on screen. However, as I progressed, I realized that writing structured and maintainable code was just as important as functionality. I started organizing materials into OBJECT_MATERIAL definitions instead of using hardcoded colors, making it easier to adjust and refine object appearances.

Additionally, I gained a deeper understanding of how shaders and lighting models work in OpenGL. Instead of relying on basic colors, I learned to adjust Phong lighting parameters to control reflections and glossiness. This was particularly useful when designing the dark green cup with a high-gloss finish, which required careful tuning of specular highlights and shininess values.

----------------------------------------------------------------------------

<b>How This Project Helps in My Future Goals</b>

Computer science, particularly in computational graphics, plays a huge role in many fields, including game development, UI/UX design, and simulations. This project introduced me to the basics of 3D rendering, which will be valuable in future work with game engines like Unity or Unreal Engine. Additionally, understanding how lighting and materials work will help in any area of programming that involves visual representation, such as data visualization, virtual reality (VR), and engineering simulations.

From an educational perspective, working on this project gave me hands-on experience with OpenGL, which is useful for computer graphics courses and more advanced programming projects. The knowledge I gained about transformations, object modeling, and shader programming can be applied in future classes related to graphics programming and software development.

----------------------------------------------------------------------------

<b>Conclusion</b>

This project was a great learning experience that improved my ability to design, develop, and refine a structured software project. I learned how to break down complex tasks into smaller, manageable pieces, how to use lighting and materials to enhance realism, and how to debug issues efficiently. My approach to coding has evolved from simply making things work to creating structured, maintainable, and visually appealing programs.

The skills I developed in OpenGL, material design, and 3D transformations will be valuable in future projects, whether in game development, engineering simulations, or interactive UI design. I now have a stronger foundation in computer graphics and visualization, which will help me as I continue to explore new challenges in programming and software development.
