// Disable warnings coming from IfcOpenShell
#pragma warning(disable:4018 4267 4250)

#include <ifcparse/IfcHierarchyHelper.h>
#include <ifcparse/Ifc4x3_add2.h>

#include <ifcgeom/abstract_mapping.h>

#include <iomanip>
#include <iostream>
#include <fstream>

#define Schema Ifc4x3_add2

// F:\IFC4.x-IF\IFC-files\Linear-placement-sleepers\ACCA\ACCA_sleepers-linear-placement-cant-implicit.ifc
// F:\IFC4.x-IF\IFC-files\Linear-placement-sleepers\ACCA\ACCA_sleepers-linear-placement-cant-explicit.ifc
// $(MSBuildProjectDirectory)\TestFiles\LargeSpirals\IFC_Alignment4_Br1.ifc
// $(MSBuildProjectDirectory)\TestFiles\HorizontalCurves\HC_N90E_Right_1b.ifc
// F:\IfcOpenshell\_build-vs2022-x64\examples\FHWA_Bridge_Geometry_Alignment_Example.ifc
// $(MSBuildProjectDirectory)\TestFiles\SpiralCurves\SC_N90E_Left_1a.ifc
// F:\IfcAlignmentDriver\TestFiles\PolynomialSpirals\ThirdOrderPolynomialSpiral.ifc
// F:\IfcAlignmentDriver\TestFiles\Helmert\GENERATED__Helmert_100.0_inf_300_1_Meter.ifc
// F:\IfcAlignmentDriver\TestFiles\I405-SR167.Flyover.ifc

void list_semantic_definition(Schema::IfcAlignment* alignment)
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
void write_placement(std::ostream& os, Schema::IfcPlacement* placement)
{
	os << placement->data().toString() << std::endl;
	if (placement->as<Schema::IfcAxis1Placement>())
	{

	}
	else if (placement->as<Schema::IfcAxis2Placement2D>())
	{
		os << placement->as<Schema::IfcAxis2Placement2D>()->Location()->data().toString() << std::endl;
		os << placement->as<Schema::IfcAxis2Placement2D>()->RefDirection()->data().toString() << std::endl;
	}
	else if (placement->as<Schema::IfcAxis2Placement3D>())
	{
		os << placement->as<Schema::IfcAxis2Placement3D>()->Location()->data().toString() << std::endl;
		os << placement->as<Schema::IfcAxis2Placement3D>()->Axis()->data().toString() << std::endl;
		os << placement->as<Schema::IfcAxis2Placement3D>()->RefDirection()->data().toString() << std::endl;
	}
	else if (placement->as<Schema::IfcAxis2PlacementLinear>())
	{
		os << placement->as<Schema::IfcAxis2PlacementLinear>()->Location()->data().toString() << std::endl;
		os << placement->as<Schema::IfcAxis2PlacementLinear>()->Axis()->data().toString() << std::endl;
		os << placement->as<Schema::IfcAxis2PlacementLinear>()->RefDirection()->data().toString() << std::endl;
	}
}

void write_line(std::ostream& os, Schema::IfcLine* line)
{
	os << line->Pnt()->data().toString() << std::endl;
	os << line->Dir()->data().toString() << std::endl;
	os << line->Dir()->Orientation()->data().toString() << std::endl;
}

void write_circle(std::ostream& os, Schema::IfcCircle* circle)
{
	os << circle->Position()->data().toString() << std::endl;
	write_placement(os, circle->Position()->as<Schema::IfcPlacement>());
}


void write_curve_segments(std::ostream& os, Schema::IfcCompositeCurve* composite_curve)
{
	auto segments = composite_curve->Segments();
	for (auto& segment : *segments)
	{
		os << segment->data().toString() << std::endl;
		auto curve_segment = segment->as<Schema::IfcCurveSegment>();
		write_placement(os, curve_segment->Placement());

		auto parent_curve = curve_segment->ParentCurve();
		os << parent_curve->data().toString() << std::endl;

		if (parent_curve->as<Schema::IfcLine>())
			write_line(os, parent_curve->as<Schema::IfcLine>());
		else if (parent_curve->as<Schema::IfcCircle>())
			write_circle(os, parent_curve->as<Schema::IfcCircle>());

	}
}

void write_curve_definition(std::ostream& os, Schema::IfcCurve* curve)
{
	if (curve->as<Schema::IfcSegmentedReferenceCurve>())
	{
		write_curve_definition(os, curve->as<Schema::IfcSegmentedReferenceCurve>()->BaseCurve());
		os << curve->as<Schema::IfcSegmentedReferenceCurve>()->data().toString() << std::endl;
		write_curve_segments(os, curve->as<Schema::IfcCompositeCurve>());
	}
	else if (curve->as<Schema::IfcGradientCurve>())
	{
		write_curve_definition(os, curve->as<Schema::IfcGradientCurve>()->BaseCurve());
		os << curve->as<Schema::IfcGradientCurve>()->data().toString() << std::endl;
		write_curve_segments(os, curve->as<Schema::IfcCompositeCurve>());
	}
	else if (curve->as<Schema::IfcCompositeCurve>())
	{
		os << curve->as<Schema::IfcCompositeCurve>()->data().toString() << std::endl;
		write_curve_segments(os, curve->as<Schema::IfcCompositeCurve>());
	}
	else if (curve->as<Schema::IfcOffsetCurveByDistances>())
	{
		os << curve->as<Schema::IfcOffsetCurveByDistances>()->data().toString() << std::endl;
	}
	else if (curve->as<Schema::IfcPolyline>())
	{
		os << curve->as<Schema::IfcPolyline>()->data().toString() << std::endl;
	}

	os << std::endl;
}

//template <typename CurveType>
void write_curve_parameters(std::ostream& os, IfcParse::IfcFile& file, ifcopenshell::geometry::abstract_mapping* mapping,const std::string& representationType = "Curve3D")
{
	auto length_unit = mapping->get_length_unit();
	auto alignments = file.instances_by_type<Schema::IfcAlignment>();
	for (auto& alignment : *alignments)
	{
		auto alignment_representation = alignment->Representation();
		auto representations = alignment_representation->Representations();
		for (auto& representation : *representations)
		{
			if (representation->RepresentationType().get_value_or("") != representationType)
				continue;

			auto items = representation->Items();
			for (auto& item : *items)
			{
				if (item->as<Schema::IfcCurve>())
				{
					auto curve = item->as<Schema::IfcCurve>();
					write_curve_definition(std::cout, curve);

					ifcopenshell::geometry::taxonomy::ptr mapped_item;
					if (curve->as<Schema::IfcSegmentedReferenceCurve>())
					{
						mapped_item = mapping->map(curve->as<Schema::IfcSegmentedReferenceCurve>());
					}
					else if (curve->as<Schema::IfcGradientCurve>())
					{
						mapped_item = mapping->map(curve->as<Schema::IfcGradientCurve>());
					}
					else if (curve->as<Schema::IfcCompositeCurve>())
					{
						mapped_item = mapping->map(curve->as<Schema::IfcCompositeCurve>());
					}
					else if (curve->as<Schema::IfcPolyline>())
					{
						mapped_item = mapping->map(curve->as<Schema::IfcPolyline>());
					}

					ifcopenshell::geometry::taxonomy::loop::ptr loop;
					auto implicit_item = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::implicit_item>(mapped_item);
					if (implicit_item)
					{
						loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(implicit_item->evaluate());
					}
					else
					{
						loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(mapped_item);
					}

					auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(implicit_item);

					os << "X, Y, u, Z, Xx, Xy, Xz, Yx, Yy, Yz, Zx, Zy, Zz" << std::endl;

					const auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->ccomponents();
					double ex = start.x(), ey = start.y();
					os << ex / length_unit << ", " << ey / length_unit << ", " << 0.0 << ", " << start.z() / length_unit;
					if (pwf)
					{
						auto p = pwf->evaluate(0.0);
						os << ", " << p.col(0)(0) << ", " << p.col(0)(1) << ", " << p.col(0)(2); // x-axis
						os << ", " << p.col(1)(0) << ", " << p.col(1)(1) << ", " << p.col(1)(2); // y-axis
						os << ", " << p.col(2)(0) << ", " << p.col(2)(1) << ", " << p.col(2)(2); // z-axis
					}
					os << std::endl;

					double u = 0;
					for (auto& edge : loop->children)
					{
						const auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(edge->end)->ccomponents();
						auto dx = e.x() - ex;
						auto dy = e.y() - ey;
						u += sqrt(dx * dx + dy * dy);
						os << e.x() / length_unit << ", " << e.y() / length_unit << ", " << u / length_unit << ", " << e.z() / length_unit;

						if (pwf)
						{
							auto p = pwf->evaluate(u);
							os << ", " << p.col(0)(0) << ", " << p.col(0)(1) << ", " << p.col(0)(2); // x-axis
							os << ", " << p.col(1)(0) << ", " << p.col(1)(1) << ", " << p.col(1)(2); // y-axis
							os << ", " << p.col(2)(0) << ", " << p.col(2)(1) << ", " << p.col(2)(2); // z-axis
						}

						os << std::endl;

						ex = e.x();
						ey = e.y();
					}
				}
			}
		}
	}

//	auto curves = file.instances_by_type<CurveType>();
//	for (auto& curve : *curves)
//	{
//		write_curve_definition(std::cout, curve->as<CurveType>());
//
//		auto mapped_item = mapping->map(curve->as<CurveType>());
//		ifcopenshell::geometry::taxonomy::loop::ptr loop;
//		auto implicit_item = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::implicit_item>(mapped_item);
//		if (implicit_item)
//		{
//		  loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(implicit_item->evaluate());
//	   }
//		else
//		{
//			loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(mapped_item);
//		}
//
//		const auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->ccomponents();
//		double ex = start.x(), ey = start.y();
//		os << start.x() / length_unit << ", " << start.y() / length_unit << ", " << 0.0 << ", " << start.z() / length_unit << std::endl;
//		double u = 0;
//		for (auto& c : loop->children)
//		{
//			const auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->ccomponents();
//			auto dx = s.x() - ex;
//			auto dy = s.y() - ey;
//			u += sqrt(dx * dx + dy * dy);
////			os << s.x()/length_unit << ", " << s.y() / length_unit << ", " << u / length_unit << ", " << s.z() / length_unit << std::endl;
//
//			const auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->ccomponents();
//			dx = e.x() - s.x();
//			dy = e.y() - s.y();
//			u += sqrt(dx * dx + dy * dy);
//			os << e.x() / length_unit << ", " << e.y() / length_unit << ", " << u / length_unit << ", " << e.z() / length_unit << std::endl;
//
//			ex = e.x();
//			ey = e.y();
//		}
//
//	}
}

int main(int argc, char** argv)
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

	ifcopenshell::geometry::Settings settings;
	settings.get<ifcopenshell::geometry::settings::PiecewiseStepType>().value = ifcopenshell::geometry::settings::PiecewiseStepMethod::MAXSTEPSIZE;
	settings.get<ifcopenshell::geometry::settings::PiecewiseStepParam>().value = 1.0;

	auto mapping = ifcopenshell::geometry::impl::mapping_implementations().construct(&file, settings);
	std::ofstream dump_file("Points.csv");

	//
	// Write out IFC elements for curve and (x,y) (u,z) coordinates
	// 
	write_curve_parameters(dump_file, file, mapping,"Curve2D");
	
	//
	// Create an offset curve
	// 

	//auto list = file.instances_by_type<Schema::IfcGradientCurve>();
	//auto gradient_curve = *(list->begin());
	//auto base_curve = gradient_curve->BaseCurve();
	//{
	//	// use 1 point, not at start of curve for uniform offset - then try multiple points with varying offsets
	//	auto offset = new Schema::IfcPointByDistanceExpression(new Schema::IfcLengthMeasure(5.0), 50.0, 100.0, boost::none, base_curve);
	//	file.addEntity(offset);
	//	typename aggregate_of<typename Schema::IfcPointByDistanceExpression>::ptr offset_values(new aggregate_of<typename Schema::IfcPointByDistanceExpression>());
	//	offset_values->push(offset);
	//	auto offset_curve = new Schema::IfcOffsetCurveByDistances(base_curve, offset_values, boost::none);
	//	file.addEntity(offset_curve);

	//	// offset curve offset from other offset curve
	//	auto offset_curve2 = new Schema::IfcOffsetCurveByDistances(offset_curve, offset_values, boost::none);
	//	file.addEntity(offset_curve2);
	//}
	//{
	//	// use 1 point, not at start of curve for uniform offset - then try multiple points with varying offsets
	//	auto offset = new Schema::IfcPointByDistanceExpression(new Schema::IfcLengthMeasure(5.0), -50.0, 100.0, boost::none, base_curve);
	//	file.addEntity(offset);
	//	typename aggregate_of<typename Schema::IfcPointByDistanceExpression>::ptr offset_values(new aggregate_of<typename Schema::IfcPointByDistanceExpression>());
	//	offset_values->push(offset);
	//	auto offset_curve = new Schema::IfcOffsetCurveByDistances(base_curve, offset_values, boost::none);
	//	file.addEntity(offset_curve);

	//	// offset curve offset from other offset curve
	//	auto offset_curve2 = new Schema::IfcOffsetCurveByDistances(offset_curve, offset_values, boost::none);
	//	file.addEntity(offset_curve2);
	//}
	////{
	////	// use 1 point, not at start of curve for uniform offset - then try multiple points with varying offsets
	////	auto offset = new Schema::IfcPointByDistanceExpression(new Schema::IfcLengthMeasure(5.0), -50.0, 100.0, boost::none, base_curve);
	////	typename aggregate_of<typename Schema::IfcPointByDistanceExpression>::ptr offset_values(new aggregate_of<typename Schema::IfcPointByDistanceExpression>());
	////	offset_values->push(offset);
	////	auto offset_curve = new Schema::IfcOffsetCurveByDistances(base_curve, offset_values, boost::none);
	////	file.addEntity(offset);
	////	file.addEntity(offset_curve);
	////}
	//std::ofstream dump_file2("OffsetPoints.csv");
	//write_curve_parameters<Schema::IfcOffsetCurveByDistances>(dump_file2, file, mapping);

	//write_curve_parameters<Schema::IfcGradientCurve>(std::cout, file, mapping);

	//auto list = file.instances_by_type<Schema::IfcGradientCurve>();
	//auto gradient_curve = *(list->begin());
	//auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::piecewise_function>(mapping->map(item->as<Schema::IfcSegmentedReferenceCurve>()));
	//std::vector<double> dist{ 0,400,425,450,500,550,750,950 };
	//for (auto d : dist)
	//{
	//	auto p = ifcopenshell::geometry::taxonomy::make<ifcopenshell::geometry::taxonomy::matrix4>(mapped_item->evaluate(d));
	//	std::cout << d << std::endl;
	//	p->print(std::cout);

	//	auto pde = new Schema::IfcPointByDistanceExpression(new Schema::IfcNonNegativeLengthMeasure(d), boost::none, boost::none, boost::none, file.instance_by_id(2617)->as<Schema::IfcCurve>());
	//	auto pl = new Schema::IfcAxis2PlacementLinear(pde, nullptr, nullptr);
	//	auto lp = new Schema::IfcLinearPlacement(nullptr, pl, nullptr);
	//	file.addEntity(lp);
	//}

	//auto pde = new Schema::IfcPointByDistanceExpression(
	//	new Schema::IfcNonNegativeLengthMeasure(425),
	//	boost::none, boost::none, boost::none,
	//	gradient_curve);

	//auto pl = new Schema::IfcAxis2PlacementLinear(pde, nullptr, nullptr);
	//auto lp = new Schema::IfcLinearPlacement(nullptr, pl, nullptr);
	//file.addEntity(lp);

	//auto placements = file.instances_by_type<Schema::IfcLinearPlacement>();
	//for (auto& object_placement : *placements)
	//{
	//	std::cout << object_placement->data().toString() << std::endl;
	//	auto m = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::matrix4>(mapping->map(object_placement));

	//	std::cout << std::fixed << std::setprecision(9);
	//	m->print(std::cout);

	//	std::cout << "det = " << m->ccomponents().determinant() << std::endl;
	//}
  

	////			auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());


	//// Write out some basic information about the alignments stored in the file
	//auto alignments = file.instances_by_type<Schema::IfcAlignment>();
	////for (auto* alignment : *alignments)
	////{
	////	std::cout << alignment->Name().get_value_or("Unnamed Alignment") << std::endl;
	////	list_semantic_definition(alignment);
	////	std::cout << std::endl;
	////	list_geometric_definition(alignment);
	////}

	////std::cout << std::endl;
	////std::cout << std::endl;

	//// Construct the geometry mapping objects
	//IfcGeom::IteratorSettings settings;
	//auto mapping = ifcopenshell::geometry::impl::mapping_implementations().construct(&file,settings);

	//////////////////////////////////////////
	//// Map a single IfcCurveSegment
	//////////////////////////////////////////

	////// Get the first IfcCurveSegment object
	////auto instance = *(file.instances_by_type<Schema::IfcCurveSegment>()->begin());

	////// map the IfcCurveSegment instance into a taxonomy::loop
	////auto item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapping->map(instance));
	////if (item)
	////{
	////	// a taxonomy::loop is a bunch of edges with start and end points. write out the start point of each edge and the end point of the last edge
	////	for (auto& c : item->children)
	////	{
	////		auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	////		std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	////	}

	////	auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(item->children.back()->end)->components();
	////	std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	////}

	//////////////////////////////////////////
	//// Map all IfcCurveSegment from the first alignment
	//////////////////////////////////////////

	////auto alignment = *(alignments->begin());
	////auto representation = *(alignment->Representation()->Representations()->begin());
	////auto composite_curve = (*(representation->Items()->begin()))->as<Schema::IfcCompositeCurve>();
	////auto curve_segments = composite_curve->Segments();
	////for (auto curve_segment : *curve_segments)
	////{
	////	auto item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapping->map(curve_segment));

	////	for (auto& c : item->children)
	////	{
	////		auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	////		std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	////	}

	////	auto& v = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(item->children.back()->end)->components();
	////	std::cout << v.x() << ", " << v.y() << ", " << v.z() << std::endl;
	////}


	//////////////////////////////////////////
	//// Map the IfcCompositeCurve for the horizontal alignment of the first alignment
	//////////////////////////////////////////

	////auto alignment = *(alignments->begin());
	////auto representations = alignment->Representation()->Representations();
	////for (auto& representation : *representations)
	////{
	////	auto items = representation->Items();
	////	for (auto& item : *items)
	////	{
	////		//std::cout << item->data().toString() << std::endl;

	////		// IfcGradientCurve and IfcSegmentReferenceCure are both IfcCompositeCurves - test for the lower level type first
	////		if (item->as<Schema::IfcGradientCurve>())
	////		{
	////			auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Schema::IfcGradientCurve>()));
	////			auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

	////			auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->components();
	////			double ex = start.x(), ey = start.y();
	////			double u = 0;
	////			for (auto& c : loop->children)
	////			{
	////				auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	////				auto dx = s.x() - ex;
	////				auto dy = s.y() - ey;
	////				u += sqrt(dx*dx + dy*dy);
	////				std::cout << s.x() << ", " << s.y() << ", " << u << ", " << s.z() << std::endl;

	////				auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
	////				dx = e.x() - s.x();
	////				dy = e.y() - s.y();
	////				u += sqrt(dx * dx + dy * dy);
	////				std::cout << e.x() << ", " << e.y() << ", " << u << ", " << e.z() << std::endl;

	////				ex = e.x();
	////				ey = e.y();
	////			}
	////		}
	////		else if (item->as<Schema::IfcSegmentedReferenceCurve>())
	////		{
	////			write_curve_definition(item->as<Schema::IfcSegmentedReferenceCurve>());

	////			auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Schema::IfcSegmentedReferenceCurve>()));
	////			auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

	////			auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->components();
	////			double ex = start.x(), ey = start.y();
	////			double u = 0;
	////			for (auto& c : loop->children)
	////			{
	////				auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	////				auto dx = s.x() - ex;
	////				auto dy = s.y() - ey;
	////				u += sqrt(dx * dx + dy * dy);
	////				std::cout << s.x() << ", " << s.y() << ", " << u << ", " << s.z() << std::endl;

	////				auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
	////				dx = e.x() - s.x();
	////				dy = e.y() - s.y();
	////				u += sqrt(dx * dx + dy * dy);
	////				std::cout << e.x() << ", " << e.y() << ", " << u << ", " << e.z() << std::endl;

	////				ex = e.x();
	////				ey = e.y();
	////			}
	////		}
	////		else if (item->as<Schema::IfcCompositeCurve>())
	////		{
	////			//write_curve_definition(item->as<Schema::IfcCompositeCurve>());

	////			auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Schema::IfcCompositeCurve>()));
	////			auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

	////			for (auto& c : loop->children)
	////			{
	////				auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	////				std::cout << s.x() << ", " << s.y() << std::endl;

	////				auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
	////				std::cout << e.x() << ", " << e.y() << std::endl;
	////			}
	////		}
	////	}
	////}

	////auto axis2_placements = file.instances_by_type<Schema::IfcAxis2PlacementLinear>();
	////for (auto& placement : *axis2_placements)
	////{
	////	auto m = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::matrix4>(mapping->map(placement));
	////	m->print(std::cout);
	////}

	////auto placements = file.instances_by_type<Schema::IfcLinearPlacement>();
	////for (auto& object_placement : *placements)
	////{
	////	auto m = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::matrix4>(mapping->map(object_placement));
	////	m->print(std::cout);
	////}

	//auto curves = file.instances_by_type<Schema::IfcSegmentedReferenceCurve>();
	//for (auto& item : *curves)
	//{
	//	write_curve_definition(item->as<Schema::IfcSegmentedReferenceCurve>());

	//	auto mapped_item = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::implicit_item>(mapping->map(item->as<Schema::IfcSegmentedReferenceCurve>()));
	//	auto loop = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::loop>(mapped_item->evaluate());

	//	auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->components();
	//	double ex = start.x(), ey = start.y();
	//	double u = 0;
	//	for (auto& c : loop->children)
	//	{
	//		auto& s = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->start)->components();
	//		auto dx = s.x() - ex;
	//		auto dy = s.y() - ey;
	//		u += sqrt(dx * dx + dy * dy);
	//		std::cout << s.x() << ", " << s.y() << ", " << u << ", " << s.z() << std::endl;

	//		auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(c->end)->components();
	//		dx = e.x() - s.x();
	//		dy = e.y() - s.y();
	//		u += sqrt(dx * dx + dy * dy);
	//		std::cout << e.x() << ", " << e.y() << ", " << u << ", " << e.z() << std::endl;

	//		ex = e.x();
	//		ey = e.y();
	//	}

	//}

   return 1;
}

//
//void list_geometric_definition(Schema::IfcAlignment* alignment)
//{
//	std::cout << "Geometric Definition of Alignments" << std::endl;
//	std::cout << alignment->data().toString() << std::endl;
//	auto* representation = alignment->Representation();
//	if (representation == nullptr)
//	{
//		std::cout << "*** Representation not provided ***" << std::endl;
//		return;
//	}
//
//	auto representations = representation->Representations();
//	for (auto& shape_representation : *representations)
//	{
//		std::cout << shape_representation->data().toString() << std::endl;
//
//		std::string representation_identifier = shape_representation->RepresentationIdentifier().has_value() ? shape_representation->RepresentationIdentifier().value() : "???";
//		if (representation_identifier == std::string("FootPrint"))
//		{
//			if (shape_representation->RepresentationType().has_value())
//				std::cout << "Representation Type (Curve2D): " << shape_representation->RepresentationType().value() << std::endl;
//			else
//				std::cout << "Representation Type (Curve2D): " << "???" << std::endl;
//
//			auto items = shape_representation->Items();
//			for (auto& item : *items)
//			{
//				std::cout << item->data().toString() << std::endl;
//
//				auto* composite_curve = item->as<Schema::IfcCompositeCurve>();
//				write_composite_curve(composite_curve);
//			}
//		}
//		else if (representation_identifier == std::string("Axis"))
//		{
//			if (shape_representation->RepresentationType().has_value())
//				std::cout << "Representation Type (Curve3D): " << shape_representation->RepresentationType().value() << std::endl;
//			else
//				std::cout << "Representation Type (Curve3D): " << "???" << std::endl;
//
//			auto items = shape_representation->Items();
//			for (auto& item : *items)
//			{
//				std::cout << item->data().toString() << std::endl;
//
//				auto* gradient_curve = item->as<Schema::IfcGradientCurve>();
//				if (gradient_curve)
//				{
//					std::cout << "Horizontal Alignment (Base Curve)" << std::endl;
//					write_composite_curve(gradient_curve->BaseCurve()->as<Schema::IfcCompositeCurve>());
//					std::cout << "Vertical Profile" << std::endl;
//					write_composite_curve(gradient_curve); // gradient_curve is an IfcCompositeCurve
//				}
//			}
//		}
//		else
//		{
//			std::cout << "Expecting FootPrint or Axis for representation identifier" << std::endl;
//		}
//	}
//}
