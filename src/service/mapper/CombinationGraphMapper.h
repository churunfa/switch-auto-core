//
// Created by churunfa on 2026/1/26.
//

#ifndef SWITCH_AUTO_CORE_COMBINATIONGRAPHMAPPER_H
#define SWITCH_AUTO_CORE_COMBINATIONGRAPHMAPPER_H
#include <google/protobuf/repeated_ptr_field.h>

#include "repo/combination/CombinationGraph.h"


namespace combination::graph {
    class CombinationEdge;
    class Combination;
    class CombinationNode;
    class CombinationGraph;
}

class CombinationGraphMapper {
public:
    static void FillNodeProto(const CombinationNode& src, combination::graph::CombinationNode* dest);
    static void FillEdgeProto(const CombinationEdge& src, combination::graph::CombinationEdge* dest);
    static void FillCombinationProto(const Combination& src, combination::graph::Combination* dest);

    static Combination buildCombination(const combination::graph::Combination& combination);
    static CombinationNode buildNode(const Combination& combination, const combination::graph::CombinationNode& node);
    static CombinationEdge buildEdge(const Combination& combination, const combination::graph::CombinationEdge& edge);
    static CombinationGraph buildGraph(const combination::graph::Combination& combination,
                           const google::protobuf::RepeatedPtrField<combination::graph::CombinationNode>& nodes,
                           const google::protobuf::RepeatedPtrField<combination::graph::CombinationEdge>& edges);
};
#endif //SWITCH_AUTO_CORE_COMBINATIONGRAPHMAPPER_H