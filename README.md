# IfcAlignmentDriver

This is an application to facilitate playing with IfcOpenShell and hopefully develop support for IfcAlignment, IfcLinearPlacement, linear referencing, etc..

I have my project setup on in `F:\IfcAlignmentDriver`. 

The following environment variables are used in the compiler and linker settings:
- `$(BOOSTDIR)` = F:\IfcOpenshell\_deps\boost_1_79_0
- `$(IFCOPENSHELL_DIR)` = F:\IfcOpenshell

# Notes

## Clothoid Curve
The documentation for the clothoid curve at https://standards.buildingsmart.org/IFC/RELEASE/IFC4_3/HTML/lexical/IfcClothoid.htm is lacking. Here are some key things that I figured out.

1. u = 1.0 when the slope of the curve is vertical. This is the point at which x starts to decrease.
2. The clothoid constant is A = sqrt(Radius x Length)
3. The limits of integration are a = 0.0 to b = (distance along curve / Length) x (u at end of curve). u at end of curve is L/(A*sqrt(PI))

