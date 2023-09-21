#pragma warning(disable:4250)

#include <ifcparse/IfcHierarchyHelper.h>
#include <ifcparse/Ifc4x3_add1.h>

//#include <ifcgeom/Iterator.h>
//#include <ifcgeom/IteratorSettings.h>

void write_composite_curve(Ifc4x3_add1::IfcCompositeCurve* composite_curve)
{
	auto segments = composite_curve->Segments();
	for (auto& segment : *segments)
	{
		std::cout << segment->data().toString() << std::endl;
		auto* curve_segment = segment->as<Ifc4x3_add1::IfcCurveSegment>();
		Ifc4x3_add1::IfcCurve* parent_curve = nullptr;
		try
		{
			parent_curve = curve_segment->ParentCurve();
		}
		catch (...)
		{
		}

		if (parent_curve)
			std::cout << parent_curve->data().toString() << std::endl;
		else
			std::cout << curve_segment->SegmentLength()->data().toString() << std::endl;
	}
}

void list_semantic_definition(boost::shared_ptr<Ifc4x3_add1::IfcAlignment::list> alignments)
{
	std::cout << "Semantic Definition of Alignments (business logic)" << std::endl;
	for (const auto& alignment : *alignments)
	{
		std::cout << alignment->data().toString() << std::endl;

		auto nested_by = alignment->IsNestedBy();
		for (const auto& nested : *nested_by)
		{
			auto related_objects = nested->RelatedObjects();
			for (const auto& related_object : *related_objects)
			{
				std::cout << related_object->data().toString() << std::endl;
			}
		}
	}
}

void list_geometric_definition(boost::shared_ptr<Ifc4x3_add1::IfcAlignment::list> alignments)
{
	std::cout << "Geometric Definition of Alignments" << std::endl;
	for (const auto& alignment : *alignments)
	{
		std::cout << alignment->data().toString() << std::endl;
		auto* representation = alignment->Representation();
		auto representations = representation->Representations();
		for (auto& shape_representation : *representations)
		{
			std::cout << shape_representation->data().toString() << std::endl;

			std::string representation_identifier = shape_representation->RepresentationIdentifier().has_value() ? shape_representation->RepresentationIdentifier().value() : "???";
			if (representation_identifier == std::string("FootPrint"))
			{
				if (shape_representation->RepresentationType().has_value())
					std::cout << "Representation Type (Curve2D): " << shape_representation->RepresentationType().value() << std::endl;
				else
					std::cout << "Representation Type (Curve2D): " << "???" << std::endl;

				auto items = shape_representation->Items();
				for (auto& item : *items)
				{
					std::cout << item->data().toString() << std::endl;

					auto* composite_curve = item->as<Ifc4x3_add1::IfcCompositeCurve>();
					write_composite_curve(composite_curve);
				}
			}
			else if (representation_identifier == std::string("Axis"))
			{
				if (shape_representation->RepresentationType().has_value())
					std::cout << "Representation Type (Curve3D): " << shape_representation->RepresentationType().value() << std::endl;
				else
					std::cout << "Representation Type (Curve3D): " << "???" << std::endl;

				auto items = shape_representation->Items();
				for (auto& item : *items)
				{
					std::cout << item->data().toString() << std::endl;

					auto* gradient_curve = item->as<Ifc4x3_add1::IfcGradientCurve>();
					std::cout << "Horizontal Alignment (Base Curve)" << std::endl;
					write_composite_curve(gradient_curve->BaseCurve()->as<Ifc4x3_add1::IfcCompositeCurve>());
					std::cout << "Vertical Profile" << std::endl;
					write_composite_curve(gradient_curve); // gradient_curve is an IfcCompositeCurve
				}
			}
			else
			{
				std::cout << "Expecting FootPrint or Axis for representaiton identifier" << std::endl;
			}
		}
	}
}

int main(int argc,char** argv)
{
	if (argc != 2) {
		std::cout << "usage: IfcOpenShellDriver <filename.ifc>" << std::endl;
		return 1;
	}

	// Redirect the output (both progress and log) to stdout
	Logger::SetOutput(&std::cout, &std::cout);

	// Parse the IFC file provided in argv[1]
	IfcParse::IfcFile file(argv[1]);
	if (!file.good()) {
		std::cout << "Unable to parse .ifc file" << std::endl;
		return 1;
	}


	auto alignments = file.instances_by_type<Ifc4x3_add1::IfcAlignment>();
   list_semantic_definition(alignments);
	std::cout << std::endl;
	list_geometric_definition(alignments);

	//std::cout << std::endl;
	//std::cout << std::endl;

	//auto alignment = *alignments->begin();
	//
	//IfcGeom::IteratorSettings settings;
	//IfcGeom::Iterator iterator(settings, &file);
	//auto base_class = iterator.create();
	//std::cout << base_class->data().toString() << std::endl;

   return 1;
}
