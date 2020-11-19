#ifndef __HOT__COMMONS__TWO_ENTRIES_NODE__
#define __HOT__COMMONS__TWO_ENTRIES_NODE__

#include <cstdint>

#include <hot/commons/SingleMaskPartialKeyMapping.hpp>
#include <hot/commons/BiNode.hpp>

namespace hot { namespace commons {

template<typename ChildPointerType, template <typename, typename> typename NodeTemplate> inline NodeTemplate<SingleMaskPartialKeyMapping, uint8_t>* createTwoEntriesNode(BiNode<ChildPointerType> const & binaryNode) {
	constexpr uint16_t NUMBER_ENTRIES_IN_TWO_ENTRIES_NODE = 2u;
	NodeTemplate<SingleMaskPartialKeyMapping, uint8_t>* node =
		new (NUMBER_ENTRIES_IN_TWO_ENTRIES_NODE) NodeTemplate<SingleMaskPartialKeyMapping, uint8_t>(
			binaryNode.mHeight,
			NUMBER_ENTRIES_IN_TWO_ENTRIES_NODE,
			SingleMaskPartialKeyMapping { DiscriminativeBit { binaryNode.mDiscriminativeBitIndex } }
		);

	node->mPartialKeys.mEntries[0] = 0;
	node->mPartialKeys.mEntries[1] = 1;
	ChildPointerType* pointers = node->getPointers();
	pointers[0] = binaryNode.mLeft;
	pointers[1] = binaryNode.mRight;
	return node;
};

} }

#endif