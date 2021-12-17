# Action Movement Component
The purpose of this project is to implement a movement wall run mechanic similar to the one found in games such as Prince of Persia, Titanfall and many others.
The implementation of the mechanic was straight forward, two line traces would be traced to the right and left of the controlling character, angled a bit backwards to be more forgiving with collisions and to provide the player with chance of looking around while wallrunning without dropping from the wallrun animation.
Once Collision is detected by these line traces, the following checks occurred:
* Check that the Hit object is perpendicular to the user (the normal.Z is between values of -0.52 and 0.52)
* Check that the player is not on the ground.
If the previous checks are found to be true the character is launched into the wall to produce a "Stick to the wall" effect, and then another launch with the forward direction is initiated, with some booleans flag set to describe the player state that then feed into the animation blueprint.

## Code Structure
Most of the code is structured within the Locomotion USceneComponent, with variables exposed to the Blueprint through use of Unreal macros. An implementation of the Vertical wall run was in place and is working but was commented from the final deliverable because of bugs within the feature itself (Works on right-side collisions but not on left-side) and because of wall jump irregular behavior, will get back to it given more time.
There was a minor change in the parent pre-defined SpringArmComponent class provided by Unreal as I added a getter and a setter to the Camera offset value to add some camera tilt effects to the movement.

### Functions
The functions are as follows:
Main :- The main function which calls our entire functionality.
HorizontalWallrunMainLoop :- The function holding the main loop for the Horizontal wall run function.
VerticalWallrunMainLoop :- The function holding the main loop for the Vertical wall run function.
GetRayCastLines :- The main function populating our class RayCast lines for collision detection.
GetHorizontalWallRunRayCast :- The function getting the RayCast lines related to horizontal wall run.
GetVerticalWallRunRayCast :- The function getting the RayCast lines related to vertical wall run.
IsPerpendicular :- Checks for perpendicularity between player and hit
CheckCollision :- Checks for collisions occurring, takes two parameters, the ray cast line which is used for hit detection and a boolean flag indicating if this is a right or left detection.
HorizontalWallrun :- Executes the horizontal wall run functionality, called by HorizontalWallrunMainLoop.
VerticalWallrun :- Executes the vertical wall run functionality, called by VerticalWallrunMainLoop.
LaunchPlayerIntoWall :- The function that produces the effect of "sticking the player to the wall" by calculating the distance between the player and the wall and launching him into it.
LaunchPlayer :- A function that takes the direction in which the player should be launched and launches him in it, in the case of the horizontal wall run the player would be launched forward, in the case of the vertical wall run the player would be launched upwards, the direction is caluclated by a cross product.
