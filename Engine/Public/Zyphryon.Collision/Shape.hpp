// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Collision.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Collision
{
    /// \brief A geometric shape that can be one of several geometry types.
    ///
    /// \tparam Types The set of geometry types that this shape can represent.
    template<typename... Types>
    class Shape final
    {
    public:

        /// \brief Creates a shape with the first geometry type default-initialized.
        ZYPHRYON_INLINE Shape() = default;

        /// \brief Constructs a shape holding the specified geometry data.
        ///
        /// \param Data The geometry data to initialize the shape with.
        template<typename Geometry>
        ZYPHRYON_INLINE explicit Shape(AnyRef<Geometry> Data)
            : mGeometry { Move(Data) }
        {
        }

        /// \brief Constructs a new geometry object in-place within the shape.
        ///
        /// \param Parameters The arguments to forward to the geometry constructor.
        /// \return A reference to the newly constructed geometry object.
        template<typename Type, typename... Arguments>
        ZYPHRYON_INLINE Ref<Type> SetData(AnyRef<Arguments>... Parameters)
        {
            return mGeometry.template Emplace<Type>(Forward<Arguments>(Parameters)...);
        }

        /// \brief Retrieves a reference to the stored geometry of the specified type.
        ///
        /// \tparam Type The type of geometry to retrieve.
        /// \return A reference to the stored geometry.
        template<typename Type>
        ZYPHRYON_INLINE Ref<Type> GetData()
        {
            return mGeometry.template GetData<Type>();
        }

        /// \brief Retrieves a const reference to the stored geometry of the specified type.
        ///
        /// \tparam Type The type of geometry to retrieve.
        /// \return A const reference to the stored geometry.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Type> GetData() const
        {
            return mGeometry.template GetData<Type>();
        }

        /// \brief Returns the index of the currently active geometry type.
        ///
        /// \return The zero-based index of the active geometry type.
        ZYPHRYON_INLINE UInt32 GetIndex() const
        {
            return mGeometry.GetIndex();
        }

        /// \brief Checks whether the shape currently holds the specified geometry type.
        ///
        /// \tparam Type The type to check for.
        /// \return `true` if the shape currently contains the specified geometry type, `false` otherwise.
        template<typename Type>
        ZYPHRYON_INLINE Bool Contains() const
        {
            return mGeometry.template Contains<Type>();
        }

        /// \brief Tests for intersection between this shape and another shape.
        ///
        /// \param Other  The other shape to test for intersection with.
        /// \param Result Optional pointer to store detailed collision manifold information.
        /// \return `true` if the shapes intersect, `false` otherwise.
        ZYPHRYON_INLINE Bool Intersects(ConstRef<Shape> Other, Ptr<Manifold> Result) const
        {
            return mGeometry.Visit([Result]<typename T0, typename T1>(ConstRef<T0> First, ConstRef<T1> Second)
            {
                return Collision::Intersects(First, Second, Result);
            }, Other.mGeometry);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mGeometry);
        }

    public:

        /// \brief Creates an anchored shape based on the source shape and pivot point.
        ///
        /// \param Source The source shape to anchor.
        /// \param Pivot  The pivot point to use for anchoring.
        /// \return A new shape anchored at the specified pivot point.
        ZYPHRYON_INLINE constexpr static Shape Anchor(ConstRef<Shape> Source, Pivot Pivot)
        {
            return (Source.mGeometry).Visit([Pivot]<typename Type>(ConstRef<Type> Data)
            {
                return Shape(Type::Anchor(Data, Pivot));
            });
        }

        /// \brief Transform the source shape using the specified transformation matrix.
        ///
        /// \param Source The source shape to project.
        /// \param Matrix The transformation matrix to apply.
        /// \return A new shape representing the projection of the source shape.
        ZYPHRYON_INLINE static Shape Transform(ConstRef<Shape> Source, ConstRef<Matrix4x4> Matrix)
        {
            return (Source.mGeometry).Visit([&]<typename Type>(ConstRef<Type> Data)
            {
                return Shape(Type::Transform(Data, Matrix));
            });
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Variant<Types...> mGeometry;
    };
}