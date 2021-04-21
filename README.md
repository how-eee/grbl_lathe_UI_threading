# grbl_lathe_UI_threading
Here is my attempt to combine two projects. 
One is User Interface for lathe Grbl on mega2560 + reprap smart display controller by bdurbrow. https://github.com/bdurbrow/grbl-Mega 
Second is threading G33 support by MetalWorkerTools. https://github.com/MetalWorkerTools/grbl-L-Mega
There is some iformation about projects on their wiki pages.

Video of components used https://youtu.be/8M4gzLkSXls
Video shows assembled dewise https://youtu.be/N7hlYoIhV2g
Short test video https://youtu.be/dcXR74F-ByY

I have changed many minor issues in bdurbrow, but I still have couple there not solved yet. Here is description and discussion https://github.com/MetalWorkerTools/grbl-L-Mega/issues/4

jog mode Axis Zeroing

Pressing PartZero button now zeroing axis in active coordinate system position, not machine position. tool_length_offset aplied if any. smartZeroDiameter applied, but I dont use it on lathe (its zero all the time) so didnot check it.

Pressing shiftedPartZeroKey zeroing Machine position with tool offset.

