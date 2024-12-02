// Disable warnings coming from IfcOpenShell
#pragma warning(disable:4018 4267 4250)

#include <ifcparse/IfcHierarchyHelper.h>
#include <ifcparse/Ifc4x3_add2.h>
#include <ifcgeom/piecewise_function_evaluator.h>
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
// F:\IfcAlignmentDriver\TestFiles\Viadotto_Acerno_ADD2_rn7jkRY.ifc

void write_point(std::ostream& os, Schema::IfcCartesianPoint* point)
{
	point->toString(os); os << std::endl;
}

void write_placement(std::ostream& os, Schema::IfcPlacement* placement)
{
	placement->toString(os); os << std::endl;
	if (placement->as<Schema::IfcAxis1Placement>())
	{

	}
	else if (placement->as<Schema::IfcAxis2Placement2D>())
	{
		placement->as<Schema::IfcAxis2Placement2D>()->Location()->toString(os); os << std::endl;
		auto ref_direction = placement->as<Schema::IfcAxis2Placement2D>()->RefDirection();
		if( ref_direction ) ref_direction->toString(os); os << std::endl;
	}
	else if (placement->as<Schema::IfcAxis2Placement3D>())
	{
		placement->as<Schema::IfcAxis2Placement3D>()->Location()->toString(os); os << std::endl;

		auto axis = placement->as<Schema::IfcAxis2Placement3D>()->Axis();
		if (axis) axis->toString(os); os << std::endl;

		auto ref_direction = placement->as<Schema::IfcAxis2Placement3D>()->RefDirection();
		if (ref_direction) ref_direction->toString(os); os << std::endl;
	}
	else if (placement->as<Schema::IfcAxis2PlacementLinear>())
	{
		placement->as<Schema::IfcAxis2PlacementLinear>()->Location()->toString(os); os << std::endl;
		
		auto axis = placement->as<Schema::IfcAxis2PlacementLinear>()->Axis();
		if(axis) axis->toString(os); os << std::endl;

		auto ref_direction = placement->as<Schema::IfcAxis2PlacementLinear>()->RefDirection();
		if (ref_direction) ref_direction->toString(os); os << std::endl;
	}
}

void write_line(std::ostream& os, Schema::IfcLine* line)
{
	line->Pnt()->toString(os); os << std::endl;
	line->Dir()->toString(os); os << std::endl;
	line->Dir()->Orientation()->toString(os); os << std::endl;
}

void write_circle(std::ostream& os, Schema::IfcCircle* circle)
{
	write_placement(os, circle->Position()->as<Schema::IfcPlacement>());
}

void write_clothoid(std::ostream& os, Schema::IfcClothoid* clothoid)
{
	write_placement(os, clothoid->Position()->as<Schema::IfcPlacement>());
}

void write_parent_curve(std::ostream& os, Schema::IfcCurve* parent_curve)
{
	parent_curve->toString(os); os << std::endl;

	if (parent_curve->as<Schema::IfcLine>())
		write_line(os, parent_curve->as<Schema::IfcLine>());
	else if (parent_curve->as<Schema::IfcCircle>())
		write_circle(os, parent_curve->as<Schema::IfcCircle>());
	else if (parent_curve->as<Schema::IfcClothoid>())
		write_clothoid(os, parent_curve->as<Schema::IfcClothoid>());
}

void write_representation(std::ostream& os, Schema::IfcRepresentation* representation)
{
	const auto citems = representation->Items();
	for (const auto item : *citems)
	{
		if (item->as<Schema::IfcCurveSegment>())
		{
            const auto segment = item->as<Schema::IfcCurveSegment>();
			segment->toString(os); os << std::endl;
			write_placement(os, segment->Placement());
			write_parent_curve(os, segment->ParentCurve());
		}
	}
}

void write_product_representation(std::ostream& os, Schema::IfcProductRepresentation* product_representation)
{
    const auto representations = product_representation->Representations();
	for (const auto representation : *representations)
	{
		write_representation(os, representation);
	}
}

void list_semantic_definition(std::ostream& os, Schema::IfcAlignmentSegment* segment)
{
    const auto design_parameters = segment->DesignParameters();
	design_parameters->toString(os); os << std::endl;
	if (design_parameters->as<Schema::IfcAlignmentHorizontalSegment>())
	{
		const auto hseg = design_parameters->as<Schema::IfcAlignmentHorizontalSegment>();
		write_point(os, hseg->StartPoint());
	}
	else if (design_parameters->as<Schema::IfcAlignmentVerticalSegment>())
	{
		auto vseg = design_parameters->as<Schema::IfcAlignmentVerticalSegment>();
		//write_point(os, vseg->StartPoint());
	}

	const auto product_representation = segment->Representation();
	write_product_representation(os, product_representation);
}

void list_semantic_definition(std::ostream& os, Schema::IfcAlignmentHorizontal* horizontal)
{
	horizontal->toString(os); os << std::endl;

	auto nested_by = horizontal->IsNestedBy();
	for (const auto& nested : *nested_by)
	{
		auto related_objects = nested->RelatedObjects();
		for (const auto& related_object : *related_objects)
		{
			related_object->toString(os); os << std::endl;
			if (related_object->as<Schema::IfcAlignmentSegment>())
			{
				list_semantic_definition(os, related_object->as<Schema::IfcAlignmentSegment>());
			}
		}
	}
}

void list_semantic_definition(std::ostream& os, Schema::IfcAlignmentVertical* vertical)
{
	vertical->toString(os); os << std::endl;

	auto nested_by = vertical->IsNestedBy();
	for (const auto& nested : *nested_by)
	{
		auto related_objects = nested->RelatedObjects();
		for (const auto& related_object : *related_objects)
		{
		   related_object->toString(os); os << std::endl;
			if (related_object->as<Schema::IfcAlignmentSegment>())
			{
				list_semantic_definition(os, related_object->as<Schema::IfcAlignmentSegment>());
			}
		}
	}
}

void list_semantic_definition(std::ostream& os, Schema::IfcAlignment* alignment)
{
	os << "Semantic Definition of Alignments (business logic)" << std::endl;
	alignment->toString(os); os << std::endl;

	auto nested_by = alignment->IsNestedBy();
	for (const auto& nested : *nested_by)
	{
		auto related_objects = nested->RelatedObjects();
		for (const auto& related_object : *related_objects)
		{
			if (related_object->as<Schema::IfcAlignmentHorizontal>())
			{
				list_semantic_definition(os, related_object->as<Schema::IfcAlignmentHorizontal>());
			}
			else if (related_object->as<Schema::IfcAlignmentVertical>())
			{
				list_semantic_definition(os, related_object->as<Schema::IfcAlignmentVertical>());
			}
		}
	}
}


void write_curve_segments(std::ostream& os, Schema::IfcCompositeCurve* composite_curve)
{
	auto segments = composite_curve->Segments();
	for (auto& segment : *segments)
	{
		segment->toString(os); os << std::endl;
		auto curve_segment = segment->as<Schema::IfcCurveSegment>();
		write_placement(os, curve_segment->Placement());

		auto parent_curve = curve_segment->ParentCurve();
		write_parent_curve(os, parent_curve);
	}
}

void write_curve_definition(std::ostream& os, Schema::IfcCurve* curve)
{
	if (curve->as<Schema::IfcSegmentedReferenceCurve>())
	{
		write_curve_definition(os, curve->as<Schema::IfcSegmentedReferenceCurve>()->BaseCurve());
		curve->as<Schema::IfcSegmentedReferenceCurve>()->toString(os); os << std::endl;
		write_curve_segments(os, curve->as<Schema::IfcCompositeCurve>());
	}
	else if (curve->as<Schema::IfcGradientCurve>())
	{
		write_curve_definition(os, curve->as<Schema::IfcGradientCurve>()->BaseCurve());
		curve->as<Schema::IfcGradientCurve>()->toString(os); os << std::endl;
		write_curve_segments(os, curve->as<Schema::IfcCompositeCurve>());
	}
	else if (curve->as<Schema::IfcCompositeCurve>())
	{
		curve->as<Schema::IfcCompositeCurve>()->toString(os); os << std::endl;
		write_curve_segments(os, curve->as<Schema::IfcCompositeCurve>());
	}
	else if (curve->as<Schema::IfcOffsetCurveByDistances>())
	{
		curve->as<Schema::IfcOffsetCurveByDistances>()->toString(os); os << std::endl;
	}
	else if (curve->as<Schema::IfcPolyline>())
	{
		curve->as<Schema::IfcPolyline>()->toString(os); os << std::endl;
	}

	os << std::endl;
}

void write_curve_parameters(IfcParse::IfcFile& file, ifcopenshell::geometry::abstract_mapping* mapping,const std::string& representationType = "Curve3D")
{
	auto length_unit = mapping->get_length_unit();
	auto alignments = file.instances_by_type<Schema::IfcAlignment>();
	int alignment_count = 0;
	for (auto& alignment : *alignments)
	{
		alignment_count++;

		auto name = alignment->Name();
		std::ostringstream oss;
		if (name.has_value())
			oss << name.get();
		else
			oss << "UnnamedAlignment" << alignment_count;

		std::ofstream osPoints(oss.str() + std::string(".csv"));
		std::ofstream osIFC(oss.str() + std::string(".txt"));

		list_semantic_definition(osIFC, alignment);

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
					//write_curve_definition(std::cout, curve);

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
					auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(implicit_item);
					ifcopenshell::geometry::piecewise_function_evaluator evaluator(pwf);
					auto points = evaluator.evaluation_points();
					if (implicit_item)
					{
						loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(evaluator.evaluate());
					}
					else
					{
						loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(mapped_item);
					}


					osPoints << "X, Y, u, Z, Xx, Xy, Xz, Yx, Yy, Yz, Zx, Zy, Zz" << std::endl;

					double d1 = pwf ? pwf->start() : 0.0;
					const auto& start = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(loop->children.begin()->get()->start)->ccomponents();
					double ex = start.x(), ey = start.y();
					osPoints << ex / length_unit << ", " << ey / length_unit << ", " << d1 << ", " << start.z() / length_unit;
					if (pwf)
					{
						auto p = evaluator.evaluate(d1);
						osPoints << ", " << p.col(0)(0) << ", " << p.col(0)(1) << ", " << p.col(0)(2); // x-axis
						osPoints << ", " << p.col(1)(0) << ", " << p.col(1)(1) << ", " << p.col(1)(2); // y-axis
						osPoints << ", " << p.col(2)(0) << ", " << p.col(2)(1) << ", " << p.col(2)(2); // z-axis
					}
					osPoints << std::endl;

					auto resolution = 0.0;
					if (pwf)
					{
						auto l = pwf->length();
						resolution = l / (loop->children.size());
					}

					double u = d1;
					for (auto& edge : loop->children)
					{
						const auto& e = boost::get<ifcopenshell::geometry::taxonomy::point3::ptr>(edge->end)->ccomponents();
						auto dx = e.x() - ex;
						auto dy = e.y() - ey;
						if (pwf)
						{
							u += resolution;
						}
						else
						{
							u += sqrt(dx * dx + dy * dy);
						}
						osPoints << e.x() / length_unit << ", " << e.y() / length_unit << ", " << u / length_unit << ", " << e.z() / length_unit;

						if (pwf)
						{
							auto p = evaluator.evaluate(u);
							osPoints << ", " << p.col(0)(0) << ", " << p.col(0)(1) << ", " << p.col(0)(2); // x-axis
							osPoints << ", " << p.col(1)(0) << ", " << p.col(1)(1) << ", " << p.col(1)(2); // y-axis
							osPoints << ", " << p.col(2)(0) << ", " << p.col(2)(1) << ", " << p.col(2)(2); // z-axis
						}

						osPoints << std::endl;

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

	//auto gc = file.instance_by_id(485)->as<Schema::IfcGradientCurve>();
	//auto mapped_item = mapping->map(gc);
	//auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(mapped_item);
	//auto [start, end] = pwf->range();
	//std::cout << "start " << start << " end " << end << std::endl;
	//auto p = pwf->evaluate(start);
	//auto length_unit = mapping->get_length_unit();
	//p /= length_unit;
	//std::cout << p(0, 3) << ", " << p(1, 3) << ", " << p(2, 3) << std::endl;

	//
	// Write out IFC elements for curve and (x,y) (u,z) coordinates
	// 
	//write_curve_parameters(file, mapping, "Curve3D");

	// map each segment
	auto ccs = file.instances_by_type<Schema::IfcCompositeCurve>();
	auto cc = (*ccs->begin())->as<Schema::IfcCompositeCurve>();
	auto segments = cc->Segments();
	auto length_unit = mapping->get_length_unit();

	for (auto segment : *segments)
	{
		auto mapped_item = mapping->map(segment);

		auto implicit_item = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::implicit_item>(mapped_item);
		auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(implicit_item);
		ifcopenshell::geometry::piecewise_function_evaluator evaluator(pwf);
		//ifcopenshell::geometry::taxonomy::loop::ptr loop = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::loop>(evaluator.evaluate());
		auto curve_segment = segment->as<Schema::IfcCurveSegment>();
		auto id = curve_segment->id();
		//double start = curve_segment->SegmentStart()->data().get_attribute_value(0);
        const double length = curve_segment->SegmentLength()->data().get_attribute_value(0);
		// Eigen::Matrix4d s = evaluator.evaluate(0.0);
		// Eigen::Matrix4d e = evaluator.evaluate(length * length_unit);

		//s(0, 3) /= length_unit;
		//s(1, 3) /= length_unit;

		//e(0, 3) /= length_unit;
		//e(1, 3) /= length_unit;
	}


	const auto gcs = file.instances_by_type<Schema::IfcGradientCurve>();
	const auto gc = (*gcs->begin())->as<Schema::IfcGradientCurve>();
	// const auto mapped_item = mapping->map(gc);
	// const auto implicit_item = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::implicit_item>(mapped_item);
	// const auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(implicit_item);
	// const ifcopenshell::geometry::piecewise_function_evaluator evaluator(pwf);
	// evaluator.evaluate(30.0);

	const auto gc_segments = gc->Segments();
	for (const auto segment : *gc_segments)
	{
		auto mapped_item = mapping->map(segment);

		auto implicit_item = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::implicit_item>(mapped_item);
		auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(implicit_item);
		ifcopenshell::geometry::piecewise_function_evaluator evaluator(pwf);

        if (const auto id = segment->id(); id == 152) {

            const double step = 160.0 * length_unit;
            std::vector<double> dist{ 0,step,2*step,3*step,4*step,5*step,6*step,7*step,8*step,9*step,10*step };
            for (auto d : dist)
            {
                const auto matrix = evaluator.evaluate(d);
                std::cout << "distance being evaluated: " << d / length_unit << std::endl;

                std::cout << "distance along base curve: " << matrix(0, 3) / length_unit << std::endl;
                std::cout << "elevation: " << matrix(1, 3) / length_unit << std::endl;
                std::cout << std::endl;
            }

	    }

	}

	//auto lp = file.instance_by_id(186676)->as<Ifc4x3_add2::IfcLinearPlacement>();
	//auto lp = file.instance_by_id(193459)->as<Ifc4x3_add2::IfcAxis2PlacementLinear>();
	//auto m = mapping->map(lp);

	//auto pde = new Schema::IfcPointByDistanceExpression(new Schema::IfcNonNegativeLengthMeasure(400.0), boost::none, boost::none, boost::none, file.instance_by_id(2603)->as<Schema::IfcCurve>());
	//auto pl = new Schema::IfcAxis2PlacementLinear(pde, nullptr, nullptr);
	//auto lp = new Schema::IfcLinearPlacement(nullptr, pl, nullptr);
	//auto m = ifcopenshell::geometry::taxonomy::cast<ifcopenshell::geometry::taxonomy::matrix4>(mapping->map(lp));

	//auto list = file.instances_by_type<Schema::IfcSegmentedReferenceCurve>();
	//auto curve = *(list->begin());
	//auto pwf = ifcopenshell::geometry::taxonomy::dcast<ifcopenshell::geometry::taxonomy::piecewise_function>(mapping->map(curve));
	//auto m4 = pwf->evaluate(400.0);

	//std::stringstream ss;
	//ss << m4;
	//std::cout << ss.str().c_str() << std::endl;

	
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
