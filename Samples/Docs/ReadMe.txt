Sample App Description
SDK 6.0

lf-camera-test/ 
Description: Demonstrates how to show a camera feed on the screen. 
Requires: webcam 
Note: This only shows the YUV camera layer. The emulator does not currently support multiple layers.
  
lf-vision-test/ 
Description: Demo using VisionMPI to detect hot spots on the screen. 
Requires: webcam 
  
lf-vision-YUV-test/ 
Description: Demo using VisionMPI to detect hot spots on the screen. Similar to lf-vision-test, but draws camera feed directly to YUV layer. 
Requires: webcam 
Note: emulation does not support mixing of YUV and RGB layers so this demoe will not run correctly in emulation
                
lf-wand-test/ 
Description: Shows how to use the light wand with VisionMPI. Once connected, press the A button to select the active controller, and press the B button to change colors. Currently, dark blue tracks the best.
Requires: Light wand and webcam
Note: Due to the way the ControllerMPI is written, the wand functionality no longer works in emulation.
  
lf-controller-test/ 
Description: Shows how to use controllers with Glasgow. 
Requires: Controller and webcam
Note: emulation only supports 1 controller

HotSpotTriggerTest/ 
Description:  VisionMPI hot spot trigger demo. Shows a programmable grid of hotspots over the camera feed.
Requires: 
Note: This does not work in emulation due to YUV and RGB mixing

AudioListener/ 
Description:  Shows an audio listener implementation
Requires: 

OpenGLMovingSprite/ 
Description:  Shows a simple OGL sprite implementation.
Requires: 
Note: Does not work in emulation due to power of 2 bug in emulation

STUB/ 
Description:  Simple stub app.
Requires: 

Video/ 
Description:  Simple video player app.
Requires: 

BUTTONS/ 
Description:  Simple button demo.
Requires: 

SetDpadBYOrientation/ 
Description:  Simple demo that shows how to use orientation.
Requires: 

TUTDemo/ 
Description:  Simple tutorial launcher demo.
Requires: 
Note: In emulation, hold down the mouse button to launch a tutorial
