import ifcopenshell
print(ifcopenshell.version)
print(ifcopenshell.__file__)
import ifcopenshell.util.placement
model = ifcopenshell.open("PGSuperExport.ifc")
placement = model.by_id(886)
print(placement)
print(ifcopenshell.util.placement.get_axis2placement(placement))
