//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2021 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../object.h"

class Mtl;

#define RENDERTIME_INSTANCING_INTERFACE Interface_ID(0x442741c3, 0x2e22675c)

namespace MaxSDK
{
    /*! This API is meant to be implemented by plugins and used by renderers to instantiate objects at render-time.
        As an example, let's say that you write an object plugin that allows users to create a new geometric primitive called SuperTeapot.
        If you implemented this API in your object plugin, and if your renderer used it, you could do things like have a single SuperTeapot in your scene
        that's rendered as 1000 teapots, each with unique transformation matrices, materials, material IDs, UVWs, etc.

        This API consists of three classes:

        - RenderTimeInstancingInterface
        - RenderInstanceSource
        - RenderInstanceTarget

        It's easier to understand how those classes are related by starting with the last one and moving up the list:

        - A RenderInstanceTarget is an instance of an INode or a Mesh. It stores all the information that describes an instance (e.g. its transform matrices,
          material override, material ID overrides, UVW overrides, etc.) except for its INode or Mesh. You will see why below.
        - A RenderInstanceSource stores an INode or a Mesh and all the RenderInstanceTargets for that particular INode or Mesh.
          In other words, a RenderInstanceSource is a container of RenderInstanceTargets (instances) that all share the same INode or Mesh.
        - A RenderTimeInstancingInterface is a container of RenderInstanceSources. This interface is implemented by a plugin (e.g. object, particle system, etc.)
          so that a renderer can access its instancing information to render it.

        If you, for example, wanted to instantiate a teapot 100 times and a box 200 times in your plugin, then:

        - Your implementation of the RenderTimeInstancingInterface would contain 2 RenderInstanceSources: one for the teapot and one for the box.
        - The RenderInstanceSource of the teapot would store a pointer to the teapot's INode and 100 RenderInstanceTargets.
        - The RenderInstanceSource of the box would store a pointer to the box's INode and 200 RenderInstanceTargets.
        - Each of the 100 RenderInstanceTargets of the teapot would contain its own transform matrices, material overrides, material ID overrides, UVW overrides, etc.
        - Each of the 200 RenderInstanceTargets of the box would contain its own transform matrices, material overrides, material ID overrides, UVW overrides, etc.

        For an example of how to implement this API in an object plugin, see the RenderTimeInstancedObject sample project.
        For an example of how to use this API in a renderer, see the description of the RenderTimeInstancingInterface class.
    */
    namespace RenderTimeInstancing
    {
        //! Forward declarations.
        enum   TypeID : int;
        class  RenderInstanceSource;
        struct MotionBlurInfo;
        struct ChannelInfo;

        /*! \brief A data channel ID.
            An opaque integer token that represents a data channel. Used to actually retrieve the channel's data.
        */
        typedef int ChannelID;

        /*! \brief The RenderTimeInstancingInterface allows a renderer to access an object's instancing information
                   so that it can do efficient instancing at render-time.
                   This interface is implemented by an object and called by a renderer.

            \note It is legal for a renderer to iterate over the RenderInstanceSources and RenderInstanceTargets from <em>multiple threads</em>.
                  However, the <em>same thread</em> should not concurrently access multiple RenderInstanceTargets at the same time.
                  This makes it legal for the object plugin to actually reuse the memory returned by 
                  the RenderInstanceSource::GetRenderInstanceTarget() method to improve performance,
                  as long as that memory block is kept separate <em>per thread</em>.

            A renderer should not call GetRenderMesh() for an object that supports this interface.
            However, even if an object implements this interface, it should ideally still implement GetRenderMesh() to return an aggregate mesh of all instances,
            so that a renderer that does <em>not</em> support this interface will at least render something.

            An instance may have custom data channels. The data channel names used by an instancer can be retrieved
            using the RenderTimeInstancingInterface::GetChannels() method. Each data channel has a name, a type, and a ChannelID, which is used to
            retrieve the actual values for each instance using the RenderInstanceTarget::GetXXX() methods.

            Usage example by a renderer:

            \code
            RenderTimeInstancingInterface* instancer = RenderTimeInstancingInterface::GetRenderTimeInstancingInterface(baseObject);
            if (instancer)
            {
                // UpdateInstanceData
                Interval valid = FOREVER;

                // Initialize motion blur info, which is used for two-way communication
                // between the renderer and the object. The renderer specifies what it wants
                // and the object responds with what it can actually accommodate
                MotionBlurInfo mblur(Interval(shutterOpen, shutterClose));
                instancer->UpdateInstanceData(t, valid, mblur, view, _T("myRenderer"));

                // Get the ChannelIDs of known data channels by using their names
                ChannelID floatChannel1  = instancer->GetChannelID(_T("myFloatChannel"),   typeFloat);
                ChannelID VectorChannel1 = instancer->GetChannelID(_T("myVectorChannel1"), typeVector);
                ChannelID VectorChannel2 = instancer->GetChannelID(_T("myVectorChannel2"), typeVector);
                ChannelID TMChannel1     = instancer->GetChannelID(_T("myTMChannel"),      typeTM);

                // Instancer acts as a container of sources. Loop over the sources in the instancer
                for (auto source : *instancer)
                {
                    auto flags = source->GetFlags();

                    // Get what will be instanced
                    void* data = source->GetData();
                    if (flags & DataFlags::df_mesh)  { ... the pointer is a mesh ... }
                    if (flags & DataFlags::df_inode) { ... the data pointer is an iNode ... }

                    // A source acts as a container of targets
                    for (auto target : *source)
                    {
                        // Deal with known data. Will return defaults if missing
                        // It is the responsibility of the caller to not call the wrong type
                        float   f1  = target->GetCustomFloat(floatChannel1);
                        Point3  v1  = target->GetCustomVector(vectorChannel1);
                        Point3  v2  = target->GetCustomVector(vectorChannel2);
                        Matrix3 tm1 = target->GetCustomVector(TMChannel1);

                        // Instance the "source" object using info from the "target"
                        // If the renderer prefers to work with velocities and spins, it should check if the object is providing such data
                        // This case is not necessary if the renderer only cares about the array of transforms
                        if (mblur.flags & MBFlags::mb_velocityspin)
                        {
                            Matrix3 tm = target->GetTM();
                            Vector3    = target->GetVelocity();
                            AngAxis    = target->GetSpin();
                            ... instance the object accordingly ...
                        }
                        else // There is no velocity/spin data, so we use transforms
                        {
                            // Gets the array of transforms over the shutter interval
                            auto tms = target->GetTMs();
                            ... instance the object accordingly ...
                        }
                    }

                    if ((flags & DataFlags::df_mesh) && (flags & DataFlags::df_pluginMustDelete))
                    {
                        ... delete the mesh ...
                    }
                }

                // Cleanup, if any is needed
                instancer->ReleaseInstanceData();
            }
            else
            {
                ... use GetRenderMesh() instead ...
            }
            \endcode
        */
        class RenderTimeInstancingInterface : public BaseInterface
        {
        public:
            /*! \name Setup, update and release */
            ///@{
            /*! \brief Make sure that the instancing data is up-to-date.
                This method needs to be called by the renderer to make sure that the instancing data is
                up-to-date and ready to be retrieved.
            @param t
                The time of the evaluation. Most often same as the shutter open time,
                if motion blur is used.
            @param valid
                Returns the validity of the returned data. For example, if FOREVER is
                returned it indicates to the renderer that the instances are not moving
                or changing at all, and could in principle be retained over multiple frames.
            @param mbinfo
                An initialized MotionBlurInfo, which upon return will contain info on how the
                motion data is returned. If the object will return values for velocity and spin,
                it should set the mb_velocityspin flag.
            @param view
                The view. This allows the object to do level-of-detail computations or
                camera frustum culling.
            @param plugin
                The plugin argument of this method takes the name of the plugin
                querying this interface, in lowercase letters.
                For example: _T("arnold"), _T("octane"), _T("redshift"), _T("vray"), etc.
                This is a somewhat arbitrary value, but by having renderers identify
                themselves during a query, the object can internally determine if any
                renderer-specific edge cases need to be processed.
                \see Struct MotionBlurInfo
            */
            virtual void UpdateInstanceData(TimeValue t, Interval& valid, MotionBlurInfo& mbinfo, const View& view, const TSTR& plugin) = 0;

            /*! \brief Release the instancing data.
                When a renderer is done with the data returned by this interface, it can call this method.
                If the generating plugin allocated some information, it can release it in this method.
            */
            virtual void ReleaseInstanceData() = 0;

            /*! \brief Get this interface's ID. */
            Interface_ID GetID() override { return RENDERTIME_INSTANCING_INTERFACE; }
            ///@}

            /*! \name Getting data channels
                These are methods that can be used to obtain the list of data channels on the object.
                Known data channels can also be requested by name.
            */
            ///@{
            //! \brief Returns a list of data channels.
            virtual MaxSDK::Array<ChannelInfo> GetChannels() const = 0;

            /*! \brief Utility method to get the ChannelID of a known data channel.
                Returns -1 if a data channel of that name and type does not exist.
            */
            virtual ChannelID GetChannelID(const TSTR& name, TypeID type) const = 0;
            ///@}

            /*! \name Getting the actual things to be instanced (the sources) */
            ///@{
            /*! \brief Get the number of sources. */
            virtual size_t GetNumInstanceSources() const = 0;

            /*! \brief Get the nth source. */
            virtual const RenderInstanceSource* GetRenderInstanceSource(size_t index) const = 0;

            //! \brief For convenience - iterator.
            class Iterator;
            //! \brief Retrieve the begin() iterator. Allows using a for (auto x : y) loop.
            Iterator begin() const { return Iterator(this); }
            //! \brief Retrieve the end() iterator.
            Iterator end() const { return Iterator(this, GetNumInstanceSources()); }

            class Iterator
            {
            public:
                Iterator(const RenderTimeInstancingInterface* item) : m_item(item), m_i(0) {}
                Iterator(const RenderTimeInstancingInterface* item, const size_t val) : m_item(item), m_i(val) {}

                Iterator&  operator++() { m_i++; return *this; }
                bool operator!=(const Iterator& iterator) { return m_i != iterator.m_i; }
                const RenderInstanceSource* operator*() { return m_item->GetRenderInstanceSource(m_i); }
            private:
                size_t m_i;
                const RenderTimeInstancingInterface* m_item;
            };
            ///@}
        };

        /*! \brief Motion blur information struct.
            This communicates information about shutter intervals and motion blur behavior between the object and the renderer.
            It is filled in and passed to the RenderTimeInstancingInterface::UpdateInstanceData() method by the renderer and the object may modify it
            to communicate back to it.
        */
        struct MotionBlurInfo
        {
            /*! \brief Defines what MotionBlurInfo::flags means. */
            enum MBFlags : signed int
            {
                mb_none = 0,              //!< \brief No flags (default).
                mb_velocityspin = 1 << 0, //!< \brief The RenderInstanceTarget::GetVelocity() and RenderInstanceTarget::GetSpin() methods will return valid values.
            };

            /*! \brief Defines information about what motion blur info is available for a source.
                \see enum MBFlags
            */
            MBFlags flags;

            /*! \brief Defines the open and closing time of the shutter.
                If set to NEVER, motion blur is not used.
            */
            Interval shutterInterval;

            /*! \brief The number of TMs that the RenderInstanceTarget::GetTMs() method should return, or -1 for a variable number of TMs.

                The renderer can use the numberOfTMs variable to specify the number of TMs that the RenderInstanceTarget::GetTMs() method should return,
                which is useful if the renderer only supports a fixed set.

                When this is passed into the RenderTimeInstancingInterface::UpdateInstanceData() method from the renderer, the meaning is as follows:
                - If this value is -1 (the default), it means "I don't care", so there can be a variable number of TMs returned.
                - If this value is 0, it means that the renderer is not rendering with motion blur, so it will only use one TM even if more than one is returned.
                - If this value is 1, it indicates that the renderer expects a single TM, so if there is any any motion blur information,
                  it needs to be handed over through the RenderInstanceTarget::GetVelocity() and RenderInstanceTarget::GetSpin() methods.
                - A value of 2 or greater means that the renderer expects exactly that many TMs, or 1 TM and velocity/spin.

                Upon returning from the RenderTimeInstancingInterface::UpdateInstanceData() method, the object can fill in the response value:
                - If this value is -1, it means that the RenderInstanceTarget::GetTMs() method will return a variable number of TMs.
                - If this value is 0, it means that the instances are not moving at all, so no motion blur needs to be calculated due to instance motion.
                  There might still be vertex motion though.
                - If this value is 1, it means that the RenderInstanceTarget::GetTMs() method will only return a single TM.
                  If there is any motion blur information, it will be handed over through the RenderInstanceTarget::GetVelocity() and
                  RenderInstanceTarget::GetSpin() methods.
                - A value of 2 or greater means that the RenderInstanceTarget::GetTMs() method will return exactly that many TMs.

                Note that the object is only allowed to modify the numberOfTMs value when:
                - The renderer passes it with a value of -1.
                - The renderer passes it with a value of 2 or greater, and the object decides to give it 1 TM and velocity/spin.
                  In this situation, the object must set numberOfTMs equal to 1 before returning.
                For all the other values, the object needs to respect the numberOfTMs requested by the renderer.
            */
            int numberOfTMs;

            MotionBlurInfo(Interval shutter = NEVER, MBFlags f = MBFlags::mb_none, int tms = -1)
            {
                shutterInterval = shutter;
                flags           = f;
                numberOfTMs     = tms;
            };
        };

        /*! \brief UVW channel override data.
            This will override all the UVW coordinates of a given map channel on a mesh with a single value.
        */
        struct InstanceUVWInfo
        {
            /*! \brief The map channel to override. */
            int channel;
            /*! \brief The UVW value to override it with. */
            UVVert value;

            /*! \brief Equals operator. */
            bool operator==(const InstanceUVWInfo& rhs) { return ((channel == rhs.channel) && (value == rhs.value)); }
            /*! \brief Not equals operator. */
            bool operator!=(const InstanceUVWInfo& rhs) { return !(*this == rhs); }
        };

        /*! \brief Material ID overrides.
            Allows mapping an existing material ID to another on an instance.
            If srcID is equal to -1, it will override all existing material IDs.
        */
        struct InstanceMatIDInfo
        {
            /*! \brief The material ID to override. If srcID is equal to -1, the override applies to all the material IDs of the mesh. */
            int srcID;
            /*! \brief The material ID that will be used to replace srcID. */
            int destID;

            /*! \brief Equals operator. */
            bool operator==(const InstanceMatIDInfo& rhs) { return ((srcID == rhs.srcID) && (destID == rhs.destID)); }
            /*! \brief Not equals operator. */
            bool operator!=(const InstanceMatIDInfo& rhs) { return !(*this == rhs); }
        };

        struct ChannelInfo
        {
            //! \brief Type of the data channel.
            enum TypeID : int
            {
                typeCustom = 0,  //!< \brief Custom data block of specified size.
                typeInt    = 1,  //!< \brief Data of type int.
                typeFloat  = 2,  //!< \brief Data of type float.
                typeVector = 3,  //!< \brief Data of type Point3.
                typeColor  = 4,  //!< \brief Data of type Color. Colors and vectors may hold semantic difference to some renderers.
                typeTM     = 5   //!< \brief Data of type Matrix3.
            };

            TSTR name;           //!< \brief The name of the data channel.
            TypeID type;         //!< \brief The type of data channel.
            ChannelID channelID; //!< \brief The data channel's ID. An opaque integer token representing the data channel. Used to actually retrieve the channel's data.
            int size;            //!< \brief For typeCustom only - the size of the data, in case the renderer needs to make a copy of it.
        };

        /*! \brief Defines what the RenderInstanceSource::GetData() method returns and how to treat it. */
        enum DataFlags : signed int
        {
            df_none             = 0,
            df_mesh             = 1 << 0, //!< \brief The RenderInstanceSource::GetData() method returns a pointer to a Mesh.
            df_inode            = 1 << 1, //!< \brief The RenderInstanceSource::GetData() method returns a pointer to an INode.
            df_pluginMustDelete = 1 << 31 //!< \brief Set if the renderer is expected to delete the data pointer after it's done using it.
        };

        /*! \brief Information about a given instance of a RenderInstanceSource. */
        class RenderInstanceTarget : public InterfaceServer
        {
        public:

            /*! \name Instance custom data access
                These methods return custom data values for each instance. Values are retrieved using ChannelIDs.
            */
            ///@{
            //! \brief Return a raw custom data pointer. If the ID is invalid, returns nullptr.
            virtual void*   GetCustomData  (ChannelID channel) const = 0;
            //! \brief Return an integer value. If the ID is invalid, returns 0.
            virtual float   GetCustomInt   (ChannelID channel) const = 0;
            //! \brief Return a float value. If the ID is invalid, returns 0.0.
            virtual float   GetCustomFloat (ChannelID channel) const = 0;
            //! \brief Return a vector value. If the ID is invalid, returns Point3(0.0,0.0,0.0).
            virtual Point3  GetCustomVector(ChannelID channel) const = 0;
            //! \brief Return a color value. If the ID is invalid, returns Color(0.0,0.0,0.0).
            virtual Color   GetCustomColor (ChannelID channel) const = 0;
            //! \brief Return a TM value. If the ID is invalid, returns Matrix().
            virtual Matrix3 GetCustomTM    (ChannelID channel) const = 0;
            ///@}

            /*! \name Instance standard data access
                These methods return the standard set of data for an instance.
            */
            ///@{
            /*! \brief Get unique instance ID.
                This method returns the unique birth ID of an instance (i.e. the birth ID
                of a particle or scattered item). This value should be unique for each
                instance in the set. It can be negative or zero.
            */
            virtual __int64 GetID() const = 0;

            /*! \brief Get user-defined Instance ID.
                This method returns the arbitrary, user-defined ID of an instance.
                Texmaps can make use of this value at render-time.
                It can be negative or zero.
            */
            virtual __int64 GetInstanceID() const = 0;

            /*! \brief Get age and lifespan (if available).
                Certain objects like particle systems will have "age" information.
                This method returns <em>true</em> if the object has these values
                and assigns the age and lifespan to the referenced parameters.
                The age value starts at zero at this instance's "birth" and increases
                up to a value of lifespan at the "death" of this instance.
                To get normalized age, divide age by lifespan.
            */
            virtual bool GetLife(float& age, float& lifespan) const = 0;

            /*! \brief Get material override.
                This method returns an instance's material override.
                A return value of nullptr means no override is set on the instance and
                thus the default node material should be used.
            */
            virtual Mtl* GetMtlOverride() const = 0;

            /*! \brief Get material ID overrides.
                This method returns an instance's material ID overrides as an array.
                An empty array is returned when no overrides have been assigned to the instance.
                A non-empty array is a list of material IDs (srcID) to be changed to
                the matching destination IDs (dstID). If srcID is equal to -1, it means <em>all</em>
                material IDs should be changed to the dstID.
            */
            virtual MaxSDK::Array<InstanceMatIDInfo> GetMatIDOverrides() const = 0;

            /*! \brief Get UVW channel overrides.
                This method returns an instace's UVW overrides for specific map channels.
                The return value is an array which contains a list of overrides
                and the map channel whose vertices they should be assigned to. An
                empty array means no UVW overrides have been assigned to the instance.
            */
            virtual MaxSDK::Array<InstanceUVWInfo> GetUVWOverrides() const = 0;
            ///@}

            /*! \name Position and motion
                These methods return the position and movement of the instance.
                Exactly what is returned can be deduced from the MotionBlurInfo struct passed to
                the RenderTimeInstancingInterface::UpdateInstanceData() method.
                Any instance motion should be computed from <em>either</em> multiple
                tranformations returned by the GetTMs() method <em>or</em> by using the single transform returned by the
                GetTM() method and applying the values returned by the GetVelocity() and GetSpin() methods on top of that.
                Both approaches should never be used at the same time. The approach of using the velocity
                and the spin should only be used if the object signals that it has this information
                using the MBData::mb_velocityspin flag. I.e. transformation matrices are
                always there but may or may not be computed <em>from</em> velocity and spin
                data. The velocity and spin is only guaranteed to be there if the flag in
                question is set.
                \note Any passed vertex velocity is in <em>addition</em> to this instance motion.
            */
            ///@{
            /*! \brief Get the transformation matrix (or matrices).

                This method returns the instance's transform(s) spread evenly over the motion
                blur interval (the interval specified by the MotionBlurInfo argument passed to
                the UpdateInstanceData method), in temporal order.

                If the array returned has a single element, it represents a static instance that is not moving.
                If it has two elements, it contains the transforms at the start and end of the interval.
                If it has three elements, it contains the transforms at the start, center, and end of the interval.
                And so on.

                A vector with more than two elements allows a renderer to compute more accurate multi-sample motion blur.
            */
            virtual MaxSDK::Array<Matrix3> GetTMs() const = 0;

            /*! \brief Get the transformation matrix at shutter open only.
                This method returns the instance's transform. This can be more efficient if the object is actually
                internally using a single transform and a velocity and spin, saving it the effort of computing
                the additional matrices. If a renderer does not plan to use multiple matrices, or if it computes motion
                blur using the GetVelocity() and GetSpin() methods, it can call this method instead of using GetTMs()[0],
                which while equivalent, might be slightly less efficient.
            */
            virtual Matrix3 GetTM() const = 0;

            /*! \brief Get instance velocity.

                Returns the instance velocity of the instance in world space, measured in units per tick.
            */
            virtual Point3 GetVelocity() const = 0;

            /*! \brief Get instance rotational velocity.

                Returns the spin of the instance, as an AngAxis in units per tick.
            */
            virtual AngAxis GetSpin() const = 0;
            ///@}
        };

        /*! \brief Information about a given source, to be instanced multiple times. */
        class RenderInstanceSource : public InterfaceServer
        {
        public:
            /*! \brief Get the flags that define the type of data stored.
                Defines the type returned by the GetData() method, and any other relevant information,
                like whether the plugin must delete the pointer once it's finished using it.
            */
            virtual DataFlags GetFlags() const = 0;

            /*! \brief Get the data pointer for the object that should be instanced.

                Currently, it is either a Mesh* or an INode*, and
                the flags returned by the GetFlags() method can be queried to find out which class type it is.
                The variable should only have one class type flag set, but may
                have other relevant information flagged as well, so one should not
                test for the class type with the equality ('==') operator, but instead with the
                bitwise AND operator ('&'). For example:

                \code
                if (flags == DataFlags::mesh)  {auto mesh = (Mesh*)data;}  // incorrect
                if (flags == DataFlags::inode) {auto node = (INode*)data;} // incorrect
                if (flags & DataFlags::mesh)   {auto mesh = (Mesh*)data;}  // correct
                if (flags & DataFlags::inode)  {auto node = (INode*)data;} // correct
                \endcode

                If the "pluginMustDelete" flag is set, the pointer should be deleted
                after use. Be sure to cast to relevant class before deletion
                so the proper destructor is called.
            */
            virtual void* GetData() const = 0;

            /*! \brief Get the velocity map channel, or -1 if none.
                This method returns the map channel where per-vertex
                velocity data (stored in units/frame) might be found, inside
                any meshes returned by the RenderTimeInstancingInterface.
                A value of -1 means that the mesh contains no per-vertex velocity data.
                \note Not all meshes are guaranteed to contain velocity data. It is
                your duty to check that this map channel is initialized on a given
                mesh and that its face count is equal to the mesh's face count.
                If both face counts are equal, you can retrieve vertex velocities
                by iterating over each face's vertices, and applying the
                corresponding map face vertex value to the vertex velocity array
                you are constructing. Vertex velocities must be indirectly retrieved
                by iterating through the faces like this, because even if the map
                vertex count is identical to the mesh vertex count, the map/mesh
                vertex indices may not correspond to each other.
                Here is an example of how vertex velocities could be retrieved from
                the velocity map channel, through a RenderInstanceSource:

                \code
                std::vector<Point3> vertexVelocities(mesh.numVerts, Point3(0,0,0));
                int velMapChan = renderInstanceSource->GetVelocityMapChannel();
                if (velMapChan >= 0 && mesh.mapSupport(velMapChan))
                {
                    MeshMap& map = mesh.maps[velMapChan];
                    if (map.fnum == mesh.numFaces)
                    {
                        for (int f = 0; f < mesh.numFaces; f++)
                        {
                            Face& meshFace = mesh.faces[f];
                            TVFace& mapFace = map.tf[f];
                            for (int v = 0; v < 3; v++)
                            {
                                int meshVInx = meshFace.v[v];
                                int mapVInx = mapFace.t[v];
                                Point3 vel = map.tv[mapVInx];
                                vertexVelocities[meshVInx] = vel;
                            }
                        }
                    }
                }
                \endcode
            */
            virtual int GetVelocityMapChannel() const = 0;

            /*! \name Access to the instance targets
            */
            ///@{
            /*! \brief Get the number of instances of this source. */
            virtual size_t GetNumInstanceTargets() const = 0;

            /*! \brief Get the nth instance of this source. */
            virtual const RenderInstanceTarget* GetRenderInstanceTarget(size_t index) const = 0;

            //! \brief For convenience - iterator.
            class Iterator;
            //! \brief Retrieve the begin() iterator. Allows using a for (auto x : y) loop.
            Iterator begin() const { return Iterator(this); }
            //! \brief Retrieve the end() iterator.
            Iterator end() const { return Iterator(this, GetNumInstanceTargets()); }

            class Iterator
            {
            public:
                Iterator(const RenderInstanceSource* item) : m_item(item), m_i(0) {}
                Iterator(const RenderInstanceSource* item, const size_t val) : m_item(item), m_i(val) {}

                Iterator& operator++() { m_i++; return *this; }
                bool operator!=(const Iterator& iterator) { return m_i != iterator.m_i; }
                const RenderInstanceTarget* operator*() { return m_item->GetRenderInstanceTarget(m_i); }
            private:
                size_t m_i;
                const RenderInstanceSource* m_item;
            };
            ///@}
        };

        inline RenderTimeInstancingInterface* GetRenderTimeInstancingInterface(BaseObject* obj)
        {
            return (RenderTimeInstancingInterface*)obj->GetInterface(RENDERTIME_INSTANCING_INTERFACE);
        }
    }
}
