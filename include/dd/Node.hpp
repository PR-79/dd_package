/*
* This file is part of the MQT DD Package which is released under the MIT license.
* See file README.md or go to https://www.cda.cit.tum.de/research/quantum_dd/ for more information.
*/

#pragma once

#include "Complex.hpp"
#include "ComplexValue.hpp"
#include "Definitions.hpp"
#include "Edge.hpp"

#include <array>
#include <cstddef>
#include <utility>

namespace dd {
    struct vNode {
        std::array<Edge<vNode>, RADIX>  e{};         // edges out of this node
        vNode*                          next{};      // used to link nodes in unique table
        std::vector<LimTable<>::Entry*> limVector{}; // a vector of pointers to lims, might be nullptr for all identity
        RefCount                        ref{};       // reference count
        Qubit                           v{};         // variable index (nonterminal) value (-1 for terminal)
        std::uint_least8_t              flags = 0;

        static vNode            terminalNode;
        constexpr static vNode* terminal{&terminalNode};

        static constexpr bool isTerminal(const vNode* p) { return p == terminal; }
    };
    using vEdge       = Edge<vNode>;
    using vCachedEdge = CachedEdge<vNode>;

    inline vNode vNode::terminalNode{{{{nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}}}, nullptr, std::vector<LimTable<>::Entry*>{}, 0U, -1, 0};

    struct mNode {
        std::array<Edge<mNode>, NEDGE>  e{};         // edges out of this node
        mNode*                          next{};      // used to link nodes in unique table
        std::vector<LimTable<>::Entry*> limVector{}; // a vector of pointers to lims, might be nullptr for all identity
        RefCount                        ref{};       // reference count
        Qubit                           v{};         // variable index (nonterminal) value (-1 for terminal)
        std::uint_least8_t              flags = 0;
        // 32 = marks a node with is symmetric.
        // 16 = marks a node resembling identity
        // 8 = marks a reduced dm node,
        // 4 = marks a dm (tmp flag),
        // 2 = mark first path edge (tmp flag),
        // 1 = mark path is conjugated (tmp flag))

        static mNode            terminalNode;
        constexpr static mNode* terminal{&terminalNode};

        static constexpr bool isTerminal(const mNode* p) { return p == terminal; }

        [[nodiscard]] inline bool isIdentity() const { return flags & 16; }
        [[nodiscard]] inline bool isSymmetric() const { return flags & 32; }
        inline void               setIdentity(bool identity) {
                          if (identity)
                flags = (flags | 16);
            else
                flags = (flags & (~16));
        }
        inline void setSymmetric(bool symmetric) {
            if (symmetric)
                flags = (flags | 32);
            else
                flags = (flags & (~32));
        }
    };
    using mEdge       = Edge<mNode>;
    using mCachedEdge = CachedEdge<mNode>;

    inline mNode mNode::terminalNode{{{{nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}}}, nullptr, std::vector<LimTable<>::Entry*>{}, 0U, -1, 32 + 16};

    struct dNode {
        std::array<Edge<dNode>, NEDGE>  e{};         // edges out of this node
        dNode*                          next{};      // used to link nodes in unique table
        std::vector<LimTable<>::Entry*> limVector{}; // a vector of pointers to lims, might be nullptr for all identity
        RefCount                        ref{};       // reference count
        Qubit                           v{};         // variable index (nonterminal) value (-1 for terminal)
        std::uint_least8_t              flags = 0;
        // 32 = marks a node with is symmetric.
        // 16 = marks a node resembling identity
        // 8 = marks a reduced dm node,
        // 4 = marks a dm (tmp flag),
        // 2 = mark first path edge (tmp flag),
        // 1 = mark path is conjugated (tmp flag))

        static dNode            terminalNode;
        constexpr static dNode* terminal{&terminalNode};
        static constexpr bool   isTerminal(const dNode* p) { return p == terminal; }

        [[nodiscard]] [[maybe_unused]] static inline bool tempDensityMatrixFlagsEqual(const std::uint_least8_t a, const std::uint_least8_t b) { return getDensityMatrixTempFlags(a) == getDensityMatrixTempFlags(b); }

        [[nodiscard]] static inline bool isConjugateTempFlagSet(const std::uintptr_t p) { return p & (1ULL << 0); }
        [[nodiscard]] static inline bool isNonReduceTempFlagSet(const std::uintptr_t p) { return p & (1ULL << 1); }
        [[nodiscard]] static inline bool isDensityMatrixTempFlagSet(const std::uintptr_t p) { return p & (1ULL << 2); }
        [[nodiscard]] static inline bool isDensityMatrixNode(const std::uintptr_t p) { return p & (1ULL << 3); }

        [[nodiscard]] [[maybe_unused]] static inline bool isConjugateTempFlagSet(const dNode* p) { return isConjugateTempFlagSet(reinterpret_cast<std::uintptr_t>(p)); }
        [[nodiscard]] [[maybe_unused]] static inline bool isNonReduceTempFlagSet(const dNode* p) { return isNonReduceTempFlagSet(reinterpret_cast<std::uintptr_t>(p)); }
        [[nodiscard]] [[maybe_unused]] static inline bool isDensityMatrixTempFlagSet(const dNode* p) { return isDensityMatrixTempFlagSet(reinterpret_cast<std::uintptr_t>(p)); }
        [[nodiscard]] [[maybe_unused]] static inline bool isDensityMatrixNode(const dNode* p) { return isDensityMatrixNode(reinterpret_cast<std::uintptr_t>(p)); }

        static inline void setConjugateTempFlagTrue(dNode*& p) { p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) | (1ULL << 0)); }
        static inline void setNonReduceTempFlagTrue(dNode*& p) { p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) | (1ULL << 1)); }
        static inline void setDensityMatTempFlagTrue(dNode*& p) { p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) | (1ULL << 2)); }
        static inline void alignDensityNode(dNode*& p) { p = reinterpret_cast<dNode*>(reinterpret_cast<std::uintptr_t>(p) & (~7ULL)); }

        [[nodiscard]] static inline std::uintptr_t getDensityMatrixTempFlags(dNode*& p) { return getDensityMatrixTempFlags(reinterpret_cast<std::uintptr_t>(p)); }
        [[nodiscard]] static inline std::uintptr_t getDensityMatrixTempFlags(const std::uintptr_t a) { return a & (7ULL); }

        void unsetTempDensityMatrixFlags() { flags = flags & (~7U); }

        inline void setDensityMatrixNodeFlag(bool densityMatrix) {
            if (densityMatrix)
                flags = (flags | 8);
            else
                flags = (flags & (~8));
        }

        static inline std::uint_least8_t alignDensityNodeNode(dNode*& p) {
            std::uint_least8_t flags = getDensityMatrixTempFlags(p);
            alignDensityNode(p);

            if (p == nullptr || p->v <= -1) {
                return 0;
            }

            if (isNonReduceTempFlagSet(flags) && !isConjugateTempFlagSet(flags)) {
                // first edge paths are not modified and the property is inherited by all child paths
                return flags;
            } else if (!isConjugateTempFlagSet(flags)) {
                // Conjugate the second edge (i.e. negate the complex part of the second edge)
                p->e[2].w.i = dd::CTEntry::flipPointerSign(p->e[2].w.i);
                setConjugateTempFlagTrue(p->e[2].p);
                // Mark the first edge
                setNonReduceTempFlagTrue(p->e[1].p);

                for (auto& edge: p->e) {
                    setDensityMatTempFlagTrue(edge.p);
                }

            } else {
                std::swap(p->e[2], p->e[1]);
                for (auto& edge: p->e) {
                    // Conjugate all edges
                    edge.w.i = dd::CTEntry::flipPointerSign(edge.w.i);
                    setConjugateTempFlagTrue(edge.p);
                    setDensityMatTempFlagTrue(edge.p);
                }
            }
            return flags;
        }

        static inline void getAlignedNodeRevertModificationsOnSubEdges(dNode* p) {
            // Before I do anything else, I must align the pointer
            alignDensityNode(p);

            for (auto& edge: p->e) {
                // remove the set properties from the node pointers of edge.p->e
                alignDensityNode(edge.p);
            }

            if (isNonReduceTempFlagSet(p->flags) && !isConjugateTempFlagSet(p->flags)) {
                // first edge paths are not modified I only have to remove the first edge property
                ;

            } else if (!isConjugateTempFlagSet(p->flags)) {
                // Conjugate the second edge (i.e. negate the complex part of the second edge)
                p->e[2].w.i = dd::CTEntry::flipPointerSign(p->e[2].w.i);

            } else {
                for (auto& edge: p->e) {
                    // Align all nodes and conjugate the weights
                    edge.w.i = dd::CTEntry::flipPointerSign(edge.w.i);
                }
                std::swap(p->e[2], p->e[1]);
            }
        }

        static inline void applyDmChangesToNode(dNode*& p) {
            // Align the node pointers
            if (isDensityMatrixTempFlagSet(p)) {
                auto tmp = alignDensityNodeNode(p);
                assert(getDensityMatrixTempFlags(p->flags) == 0);
                p->flags = p->flags | tmp;
            }
        }

        static inline void revertDmChangesToNode(dNode*& p) {
            // Align the node pointers
            if (isDensityMatrixTempFlagSet(p->flags)) {
                getAlignedNodeRevertModificationsOnSubEdges(p);
                p->unsetTempDensityMatrixFlags();
            }
        }
    };
    using dEdge       = Edge<dNode>;
    using dCachedEdge = CachedEdge<dNode>;
    inline dNode dNode::terminalNode{{{{nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}, {nullptr, Complex::zero, nullptr}}}, nullptr, std::vector<LimTable<>::Entry*>{}, 0, -1, 0};

//    template<typename Node>
    inline std::ostream& operator<<(std::ostream& out, const vNode& node) {
    	if (node.v < 0) {
    		return out << "Terminal node";
    	}
    	return out << "Node on " << node.v + 1 << " qubits."
			<< "  {low = "  << node.e[0].w << " * " << LimEntry<>::to_string(node.e[0].l, node.e[0].p->v) << "}"
			<< "  {high = " << node.e[1].w << " * " << LimEntry<>::to_string(node.e[1].l, node.e[1].p->v) << "}";
    }

    static inline dEdge densityFromMatrixEdge(const mEdge& e) {
        return dEdge{reinterpret_cast<dNode*>(e.p), e.w, e.l};
    }

} // namespace dd

namespace std {
    template<>
    struct hash<dd::dEdge> {
        std::size_t operator()(dd::dEdge const& e) const noexcept {
            const auto h1 = dd::murmur64(reinterpret_cast<std::size_t>(e.p));
            const auto h2 = std::hash<dd::Complex>{}(e.w);
            assert(e.p != nullptr);
            assert((dd::dNode::isDensityMatrixTempFlagSet(e.p)) == false);
            const auto h3  = std::hash<std::uint_least8_t>{}(dd::dNode::getDensityMatrixTempFlags(e.p->flags));
            const auto tmp = dd::combineHash(h1, h2);
            return dd::combineHash(tmp, h3);
        }
    };
} // namespace std
