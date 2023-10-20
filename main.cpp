// Disable warnings coming from IfcOpenShell
#pragma warning(disable:4018 4267 4250)

#include <ifcparse/IfcHierarchyHelper.h>
#include <ifcparse/Ifc4x3_add1.h>

#include <ifcgeom/abstract_mapping.h>


void list_semantic_definition(Ifc4x3_add1::IfcAlignment* alignment)
{
	std::cout << "Semantic Definition of Alignments (business logic)" << std::endl;
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
			std::cout << "Parent Curve: " << parent_curve->data().toString() << std::endl;
		else
			std::cout << "Zero length: " << curve_segment->SegmentLength()->data().toString() << std::endl;
	}
}

void list_geometric_definition(Ifc4x3_add1::IfcAlignment* alignment)
{
	std::cout << "Geometric Definition of Alignments" << std::endl;
	std::cout << alignment->data().toString() << std::endl;
	auto* representation = alignment->Representation();
	if (representation == nullptr)
	{
		std::cout << "*** Representation not provided ***" << std::endl;
		return;
	}

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
				if (gradient_curve)
				{
					std::cout << "Horizontal Alignment (Base Curve)" << std::endl;
					write_composite_curve(gradient_curve->BaseCurve()->as<Ifc4x3_add1::IfcCompositeCurve>());
					std::cout << "Vertical Profile" << std::endl;
					write_composite_curve(gradient_curve); // gradient_curve is an IfcCompositeCurve
				}
			}
		}
		else
		{
			std::cout << "Expecting FootPrint or Axis for representation identifier" << std::endl;
		}
	}
}

int main(int argc,char** argv)
{
	if (argc != 2) {
		std::cout << "usage: IfcAlignmentDriver <filename.ifc>" << std::endl;
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

	// Write out some basic information about the alignments stored in the file
	auto alignments = file.instances_by_type<Ifc4x3_add1::IfcAlignment>();
	//for (auto* alignment : *alignments)
	//{
	//	std::cout << alignment->Name().get_value_or("Unnamed Alignment") << std::endl;
	//	list_semantic_definition(alignment);
	//	std::cout << std::endl;
	//	list_geometric_definition(alignment);
	//}

	//std::cout << std::endl;
	//std::cout << std::endl;

	// Construct the geometry mapping objects
	IfcGeom::IteratorSettings settings;
	auto mapping = ifcopenshell::geometry::impl::mapping_implementations().construct(&file,settings);

	////////////////////////////////////////
	// Map a single IfcCurveSegment
	////////////////////////////////////////

	//// Get the first IfcCurveSegment object
	//auto instance = *(file.instances_by_type<Ifc4x3_add1::IfcCurveSegment>()->begin());

	//// map the IfcCurveSegment instance into a taxonomy::loop
	//auto item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapping->map(instance));
	//if (item)
	//{
	//	// a taxonomy::loop is a bunch of edges with start and end points. write out the start point of each edge and the end point of the last edge
	//	for (auto& c : item->children)
	//	{
	//		auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	//		std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	//	}

	//	auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(item->children.back()->end)->components();
	//	std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	//}

	////////////////////////////////////////
	// Map all IfcCurveSegment from the first alignment
	////////////////////////////////////////

	//auto alignment = *(alignments->begin());
	//auto representation = *(alignment->Representation()->Representations()->begin());
	//auto composite_curve = (*(representation->Items()->begin()))->as<Ifc4x3_add1::IfcCompositeCurve>();
	//auto curve_segments = composite_curve->Segments();
	//for (auto curve_segment : *curve_segments)
	//{
	//	auto item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapping->map(curve_segment));

	//	for (auto& c : item->children)
	//	{
	//		auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	//		std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	//	}

	//	auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(item->children.back()->end)->components();
	//	std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	//}


	////////////////////////////////////////
	// Map the IfcCompositeCurve for the horizontal alignment of the first alignment
	////////////////////////////////////////

	auto alignment = *(alignments->begin());
	auto representations = alignment->Representation()->Representations();
	for (auto& representation : *representations)
	{
		auto items = representation->Items();
		for (auto& item : *items)
		{
			std::cout << item->data().toString() << std::endl;

			// IfcGradientCurve and IfcSegmentReferenceCure are both IfcCompositeCurves - test for the lower level type first
			if (item->as<Ifc4x3_add1::IfcGradientCurve>())
			{
				auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Ifc4x3_add1::IfcGradientCurve>()));
				auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

				auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->components();
				double ex = start.x(), ey = start.y();
				double u = 0;
				for (auto& c : loop->children)
				{
					auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
					auto dx = s.x() - ex;
					auto dy = s.y() - ey;
					u += sqrt(dx*dx + dy*dy);
					std::cout << s.x() << ", " << s.y() << ", " << u << ", " << s.z() << std::endl;

					auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
					dx = e.x() - s.x();
					dy = e.y() - s.y();
					u += sqrt(dx * dx + dy * dy);
					std::cout << e.x() << ", " << e.y() << ", " << u << ", " << e.z() << std::endl;

					ex = e.x();
					ey = e.y();
				}
			}
			else if (item->as<Ifc4x3_add1::IfcSegmentedReferenceCurve>())
			{
				//auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Ifc4x3_add1::IfcSegmentedReferenceCurve>()));
				//auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

				//auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->components();
				//double ex = start.x(), ey = start.y();
				//double u = 0;
				//for (auto& c : loop->children)
				//{
				//	auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
				//	auto dx = s.x() - ex;
				//	auto dy = s.y() - ey;
				//	u += sqrt(dx * dx + dy * dy);
				//	std::cout << s.x() << ", " << s.y() << ", " << u << ", " << s.z() << std::endl;

				//	auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
				//	dx = e.x() - s.x();
				//	dy = e.y() - s.y();
				//	u += sqrt(dx * dx + dy * dy);
				//	std::cout << e.x() << ", " << e.y() << ", " << u << ", " << e.z() << std::endl;

				//	ex = e.x();
				//	ey = e.y();
				//}
			}
			else if (item->as<Ifc4x3_add1::IfcCompositeCurve>())
			{
				auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Ifc4x3_add1::IfcCompositeCurve>()));
				auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

				for (auto& c : loop->children)
				{
					auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
					std::cout << s.x() << ", " << s.y() << std::endl;

					auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
					std::cout << e.x() << ", " << e.y() << std::endl;
				}
			}
		}
	}

	//auto axis2_placements = file.instances_by_type<Ifc4x3_add1::IfcAxis2PlacementLinear>();
	//for (auto& placement : *axis2_placements)
	//{
	//	auto m = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::matrix4>(mapping->map(placement));
	//	m->print(std::cout);
	//}

	auto placements = file.instances_by_type<Ifc4x3_add1::IfcLinearPlacement>();
	for (auto& object_placement : *placements)
	{
		auto m = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::matrix4>(mapping->map(object_placement));
		m->print(std::cout);
	}

   return 1;
}
