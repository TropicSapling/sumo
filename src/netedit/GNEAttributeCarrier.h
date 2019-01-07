/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2018 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    GNEAttributeCarrier.h
/// @author  Jakob Erdmann
/// @date    Mar 2011
/// @version $Id$
///
// Abstract Base class for gui objects which carry attributes
/****************************************************************************/
#ifndef GNEAttributeCarrier_h
#define GNEAttributeCarrier_h


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/gui/images/GUIIcons.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include <utils/xml/SUMOXMLDefinitions.h>

#include "GNEReferenceCounter.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GNENet;
class GNEUndoList;
class GNELane;
class GNEEdge;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEAttributeCarrier
 *
 * Abstract Base class for gui objects which carry attributes
 * inherits from GNEReferenceCounter for convenience
 */
class GNEAttributeCarrier : public GNEReferenceCounter {
    /// @brief declare friend class
    friend class GNEChange_Attribute;

public:
    /// @brief struct with the tag Properties
    enum AttrProperty {
        ATTRPROPERTY_INT =          1 << 0,     // Attribute is an integer (Including Zero)
        ATTRPROPERTY_FLOAT =        1 << 1,     // Attribute is a float
        ATTRPROPERTY_BOOL =         1 << 2,     // Attribute is boolean (0/1, true/false)
        ATTRPROPERTY_STRING =       1 << 3,     // Attribute is a string
        ATTRPROPERTY_POSITION =     1 << 4,     // Attribute is a position defined by doubles (x,y or x,y,z)
        ATTRPROPERTY_COLOR =        1 << 5,     // Attribute is a color defined by a specifically word (Red, green) or by a special format (XXX,YYY,ZZZ)
        ATTRPROPERTY_VCLASS =       1 << 6,     // Attribute is a VClass (passenger, bus, motorcicle...)
        ATTRPROPERTY_POSITIVE =     1 << 7,     // Attribute is positive (Including Zero)
        ATTRPROPERTY_NOTZERO =      1 << 8,     // Attribute cannot be 0 (only for numerical attributes)
        ATTRPROPERTY_UNIQUE =       1 << 9,     // Attribute is unique (cannot be edited in a selection of similar elements (ID, Position...)
        ATTRPROPERTY_FILENAME =     1 << 10,    // Attribute is a filename (string that cannot contains certain characters)
        ATTRPROPERTY_NONEDITABLE =  1 << 11,    // Attribute is non editable (index of a lane)
        ATTRPROPERTY_DISCRETE =     1 << 12,    // Attribute is discrete (only certain values are allowed)
        ATTRPROPERTY_PROBABILITY =  1 << 13,    // Attribute is probability (only allowed values between 0 and 1, including both)
        ATTRPROPERTY_TIME =         1 << 14,    // Attribute is a Time (float positive)
        ATTRPROPERTY_ANGLE =        1 << 15,    // Attribute is an angle (only takes values between 0 and 360, including both, another value will be automatically reduced
        ATTRPROPERTY_LIST =         1 << 16,    // Attribute is a list of other elements separated by spaces
        ATTRPROPERTY_SECUENCIAL =   1 << 17,    // Attribute is a special sequence of elements (for example: secuencial lanes in Multi Lane E2 detectors)
        ATTRPROPERTY_OPTIONAL =     1 << 18,    // Attribute is optional
        ATTRPROPERTY_DEFAULTVALUE = 1 << 19,    // Attribute owns a default value
        ATTRPROPERTY_COMBINABLE =   1 << 20,    // Attribute is combinable with other Attribute
        ATTRPROPERTY_SYNONYM =      1 << 21,    // Attribute will be written with a different name in der XML
        ATTRPROPERTY_RANGE =        1 << 22,    // Attribute only accept a range of elements
    };

    /// @brief struct with the attribute Properties
    class AttributeProperties {
    public:
        /// @brief default constructor
        AttributeProperties();

        /// @brief parameter constructor
        AttributeProperties(int attributeProperty, int positionListed, const std::string& definition, const std::string& defaultValue, const std::vector<std::string>& discreteValues, SumoXMLAttr synonym, double minimum = 0, double maximum = 0);

        /// @brief destructor
        ~AttributeProperties();

        /// @brief check Attribute integrity (For example, throw an exception if tag has a Float default value, but given default value cannot be parse to float)
        void checkAttributeIntegrity();

        /// @brief get position in list (used in frames for listing attributes with certain sort)
        int getPositionListed() const;

        /// @brief get default value
        const std::string& getDefinition() const;

        /// @brief get default value
        const std::string& getDefaultValue() const;

        /// @brief return a description of attribute
        std::string getDescription() const;

        /// @brief get discrete values
        const std::vector<std::string>& getDiscreteValues() const;

        /// @brief get tag synonym
        SumoXMLAttr getAttrSynonym() const;

        /// @brief get minimum range
        double getMinimumRange() const;

        /// @brief get maximum range
        double getMaximumRange() const;

        /// @brief return true if attribute owns a default value
        bool hasDefaultValue() const;

        /// @brief return true if Attr correspond to an element that will be written in XML with another name
        bool hasAttrSynonym() const;

        /// @brief return true if Attr correspond to an element that only accept a range of values
        bool hasAttrRange() const;

        /// @brief return true if atribute is an integer
        bool isInt() const;

        /// @brief return true if atribute is a float
        bool isFloat() const;

        /// @brief return true if atribute is boolean
        bool isBool() const;

        /// @brief return true if atribute is a string
        bool isString() const;

		/// @brief return true if atribute is a position
        bool isposition() const;

        /// @brief return true if atribute is a probability
        bool isProbability() const;

        /// @brief return true if atribute is numerical (int or float)
        bool isNumerical() const;

        /// @brief return true if atribute is time
        bool isTime() const;

        /// @brief return true if atribute is positive
        bool isPositive() const;

        /// @brief return true if atribute cannot be zero
        bool cannotBeZero() const;

        /// @brief return true if atribute is a color
        bool isColor() const;

        /// @brief return true if atribute is a filename
        bool isFilename() const;

        /// @brief return true if atribute is a VehicleClass
        bool isVClass() const;

        /// @brief return true if atribute is a VehicleClass
        bool isSVCPermission() const;

        /// @brief return true if atribute is a list
        bool isList() const;

        /// @brief return true if atribute is sequential
        bool isSecuential() const;

        /// @brief return true if atribute is unique
        bool isUnique() const;

        /// @brief return true if atribute is optional
        bool isOptional() const;

        /// @brief return true if atribute is discrete
        bool isDiscrete() const;

        /// @brief return true if atribute is combinable with other Attribute
        bool isCombinable() const;

        /// @brief return true if atribute isn't editable
        bool isNonEditable() const;

    private:
        /// @brief Property of attribute
        int myAttributeProperty;

        /// @brief listed position
        int myPositionListed;

        /// @brief text with a definition of attribute
        std::string myDefinition;

        /// @brief default value (by default empty)
        std::string myDefaultValue;

        /// @brief discrete values that can take this Attribute (by default empty)
        std::vector<std::string> myDiscreteValues;

        /// @brief Attribute written in XML (If is SUMO_ATTR_NOTHING), original Attribute will be written)
        SumoXMLAttr myAttrSynonym;

        /// @brief minimun Range
        double myMinimumRange;

        /// @brief maxium Range
        double myMaximumRange;
    };

    enum TAGProperty {
        TAGPROPERTY_NETELEMENT =          1 << 0,   // Edges, Junctions, Lanes...
        TAGPROPERTY_ADDITIONAL =          1 << 1,   // Bus Stops, Charging Stations, Detectors...
        TAGPROPERTY_SHAPE =               1 << 2,   // POIs, Polygons
        TAGPROPERTY_TAZ =                 1 << 3,   // Traffic Assignment Zones
        TAGPROPERTY_STOPPINGPLACE =       1 << 4,   // StoppingPlaces (BusStops, ChargingStations...)
        TAGPROPERTY_DETECTOR =            1 << 5,   // Detectors (E1, E2...)
        TAGPROPERTY_ROUTEELEMENT =        1 << 6,   // VTypes, Vehicles, Flows...
        TAGPROPERTY_DRAWABLE =            1 << 7,   // Element can be drawed in view
        TAGPROPERTY_BLOCKMOVEMENT =       1 << 8,   // Element can block their movement
        TAGPROPERTY_BLOCKSHAPE =          1 << 9,   // Element can block their shape
        TAGPROPERTY_CLOSESHAPE =          1 << 10,  // Element can close their shape
        TAGPROPERTY_GEOPOSITION =         1 << 11,  // Element's position can be defined using a GEO position
        TAGPROPERTY_GEOSHAPE =            1 << 12,  // Element's shape acn be defined using a GEO Shape
        TAGPROPERTY_DIALOG =              1 << 13,  // Element can be edited using a dialog (GNECalibratorDialog, GNERerouterDialog...)
        TAGPROPERTY_PARENT =              1 << 14,  // Element will be writed in XML as child of another element (E3Entry -> E3Detector...)
        TAGPROPERTY_MINIMUMCHILDS =       1 << 15,  // Element will be only writed in XML if has a minimum number of childs
        TAGPROPERTY_REPARENT =            1 << 16,  // Element can be reparent
        TAGPROPERTY_SYNONYM =             1 << 17,  // Element will be written with a different name in der XML
        TAGPROPERTY_AUTOMATICSORTING =    1 << 18,  // Element sort automatic their Childs (used by Additionals)
        TAGPROPERTY_SELECTABLE =          1 << 19,  // Element is selectable
        TAGPROPERTY_MASKSTARTENDPOS =     1 << 20,  // Element mask attributes StartPos and EndPos as "lenght" (Only used in the appropiate GNEFrame)
        TAGPROPERTY_MASKXYZPOSITION =     1 << 21,  // Element mask attributes X, Y and Z as "Position"
        TAGPROPERTY_WRITECHILDSSEPARATE = 1 << 22,  // Element writes their childs in a separated filename
        TAGPROPERTY_PLACEDOVER_VIEW =     1 << 23,  // Element will be placed in view
        TAGPROPERTY_PLACEDOVER_EDGE =     1 << 24,  // Element will be placed over an edge
        TAGPROPERTY_PLACEDOVER_LANE =     1 << 25,  // Element will be placed over a lane
        TAGPROPERTY_PLACEDOVER_JUNCTION = 1 << 26,  // Element will be placed over a junction
        TAGPROPERTY_PLACEDOVER_EDGES =    1 << 27,  // Element will be placed over a list of edges
        TAGPROPERTY_PLACEDOVER_LANES =    1 << 28,  // Element will be placed over a list of lanes
        TAGPROPERTY_NOGENERICPARAMETERS = 1 << 29,  // Element doesn't accept Generic Parameters (by default all tags supports generic parameters)
    };

    /// @brief struct with the attribute Properties
    class TagProperties {
    public:
        /// @brief default constructor
        TagProperties();

        /// @brief parameter constructor
        TagProperties(SumoXMLTag tag, int tagProperty, int &positionListed, GUIIcon icon, SumoXMLTag parentTag = SUMO_TAG_NOTHING, SumoXMLTag tagSynonym = SUMO_TAG_NOTHING);

        /// @brief destructor
        ~TagProperties();
  
        /// @brief get Tag vinculated with this attribute Property
        SumoXMLTag getTag() const;
        
        /// @brief get Tag vinculated with this attribute Property in String Format (used to avoid multiple calls to toString(...)
        const std::string &getTagStr() const;

        /// @brief check Tag integrity (this include all their attributes)
        void checkTagIntegrity() const;

        /// @brief add attribute (duplicated attributed aren't allowed)
        void addAttribute(SumoXMLAttr attr, const int attributeProperty, const std::string& definition, const std::string& defaultValue, std::vector<std::string> discreteValues = std::vector<std::string>(), SumoXMLAttr synonym = SUMO_ATTR_NOTHING);

        /// @brief add attribute with synonym (duplicated attributed aren't allowed)
        void addAttribute(SumoXMLAttr attr, const int attributeProperty, const std::string& definition, const std::string& defaultValue, SumoXMLAttr synonym);

        /// @brief add attribute with a range
        void addAttribute(SumoXMLAttr attr, const int attributeProperty, const std::string& definition, const std::string& defaultValue, double minimum, double maximum);

        /// @brief add deprecated Attribute
        void addDeprecatedAttribute(SumoXMLAttr attr);

        /// @brief get attribute (throw error if doesn't exist)
        const AttributeProperties& getAttributeProperties(SumoXMLAttr attr) const;

        /// @brief get begin of attribute values (used for iterate)
        std::map<SumoXMLAttr, AttributeProperties>::const_iterator begin() const;

        /// @brief get end of attribute values (used for iterate)
        std::map<SumoXMLAttr, AttributeProperties>::const_iterator end() const;

        /// @brief get number of attributes
        int getNumberOfAttributes() const;

        /// @brief return the default value of the attribute of an element
        const std::string& getDefaultValue(SumoXMLAttr attr) const;

        /// @brief get GUI icon associated to this Tag
        GUIIcon getGUIIcon() const;

        /// @brief get position in list (used in frames for listing tags with certain sort)
        int getPositionListed() const;

        /// @brief if Tag owns a parent, return parent tag
        SumoXMLTag getParentTag() const;

        /// @brief get tag synonym
        SumoXMLTag getTagSynonym() const;

        /// @brief check if current TagProperties owns the attribute attr
        bool hasAttribute(SumoXMLAttr attr) const;

        /// @brief return true if tag correspond to a netElement
        bool isNetElement() const;

        /// @brief return true if tag correspond to an additional
        bool isAdditional() const;

        /// @brief return true if tag correspond to a shape
        bool isShape() const;

        /// @brief return true if tag correspond to a TAZ
        bool isTAZ() const;

        /// @brief return true if tag correspond to a detector (Only used to group all stoppingPlaces in the output XML)
        bool isStoppingPlace() const;

        /// @brief return true if tag correspond to a shape (Only used to group all detectors in the XML)
        bool isDetector() const;

        /// @brief return true if tag correspond to a drawable element
        bool isDrawable() const;

        /// @brief return true if tag correspond to a selectable element
        bool isSelectable() const;

        /// @brief return true if tag correspond to an element that can block their movement
        bool canBlockMovement() const;

        /// @brief return true if tag correspond to an element that can block their shape
        bool canBlockShape() const;

        /// @brief return true if tag correspond to an element that can close their shape
        bool canCloseShape() const;

        /// @brief return true if tag correspond to an element that can use a geo position
        bool hasGEOPosition() const;

        /// @brief return true if tag correspond to an element that can use a geo shape
        bool hasGEOShape() const;

        /// @brief return true if tag correspond to an element that can had another element as parent
        bool hasParent() const;

        /// @brief return true if tag correspond to an element that will be written in XML with another tag
        bool hasTagSynonym() const;

        /// @brief return true if tag correspond to an element that can be edited using a dialog
        bool hasDialog() const;

        /// @brief return true if tag correspond to an element that only have a limited number of childs
        bool hasMinimumNumberOfChilds() const;

        /// @brief return true if Tag correspond to an element that supports generic parameters
        bool hasGenericParameters() const;

        /// @brief return true if tag correspond to an element that can be reparent
        bool canBeReparent() const;

        /// @brief return true if tag correspond to an element that can sort their childs automatic
        bool canAutomaticSortChilds() const;

        /// @brief return true if tag correspond to an element that can sort their childs automatic
        bool canWriteChildsSeparate() const;

        /// @brief return true if tag correspond to an element that can be placed over the view
        bool canBePlacedOverView() const;

        /// @brief return true if tag correspond to an element that can be placed over an edge
        bool canBePlacedOverEdge() const;

        /// @brief return true if tag correspond to an element that can be placed over a lane
        bool canBePlacedOverLane() const;

        /// @brief return true if tag correspond to an element that can be placed over a junction
        bool canBePlacedOverJunction() const;

        /// @brief return true if tag correspond to an element that can be placed over a list of edges
        bool canBePlacedOverEdges() const;

        /// @brief return true if tag correspond 
        bool canBePlacedOverLanes() const;

        /// @brief return true if tag correspond to an element that can mask the attributes "start" and "end" position as attribute "lenght"
        bool canMaskStartEndPos() const;

        /// @brief return true if tag correspond to an element that can mask the attributes "X", "Y" and "Z" position as attribute "Position"
        bool canMaskXYZPositions() const;

        /// @brief return true if attribute of this tag is deprecated
        bool isAttributeDeprecated(SumoXMLAttr attr) const;

    private:
        /// @brief Sumo XML Tag vinculated wit this tag Property
        SumoXMLTag myTag;

        /// @brief Sumo XML Tag vinculated wit this tag Property in String format
        std::string myTagStr;

        /// @brief Property of attribute
        int myTagProperty;

        /// @brief map with the attribute values vinculated with this Tag
        std::map<SumoXMLAttr, AttributeProperties> myAttributeProperties;

        /// @brief icon associated to this Tag
        GUIIcon myIcon;

        /// @brief listed position
        int myPositionListed;

        /// @brief parent tag
        SumoXMLTag myParentTag;

        /// @brief Tag written in XML (If is SUMO_TAG_NOTHING), original Tag name will be written)
        SumoXMLTag myTagSynonym;

        /// @brief List with the deprecated Attributes
        std::vector<SumoXMLAttr> myDeprecatedAttributes;
    };

    /**@brief Constructor
     * @param[in] tag SUMO Tag assigned to this type of object
     * @param[in] icon GUIIcon associated to the type of object
     */
    GNEAttributeCarrier(SumoXMLTag tag);

    /// @brief Destructor
    virtual ~GNEAttributeCarrier() {};

    /// @name This functions has to be implemented in all GNEAttributeCarriers
    /// @{
    /// @brief select attribute carrier using GUIGlobalSelection
    virtual void selectAttributeCarrier(bool changeFlag = true) = 0;

    /// @brief unselect attribute carrier using GUIGlobalSelection
    virtual void unselectAttributeCarrier(bool changeFlag = true) = 0;

    /// @brief check if attribute carrier is selected
    virtual bool isAttributeCarrierSelected() const = 0;

    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    virtual std::string getAttribute(SumoXMLAttr key) const = 0;

    /* @brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     * @param[in] net optionally the GNENet to inform about gui updates
     */
    virtual void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) = 0;

    /* @brief method for setting the attribute and letting the object perform additional changes
    * @param[in] key The attribute key
    * @param[in] value The new value
    * @param[in] undoList The undoList on which to register changes
    */
    virtual bool isValid(SumoXMLAttr key, const std::string& value) = 0;

    /// @brief get PopPup ID (Used in AC Hierarchy)
    virtual std::string getPopUpID() const = 0;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    virtual std::string getHierarchyName() const = 0;
    /// @}

    /// @name Certain attributes and ACs (for example, connections) can be either loaded or guessed. The following static variables are used to remark it.
    /// @{
    /// @brief feature is still unchanged after being loaded (implies approval)
    static const std::string FEATURE_LOADED;

    /// @brief feature has been reguessed (may still be unchanged be we can't tell (yet)
    static const std::string FEATURE_GUESSED;

    /// @brief feature has been manually modified (implies approval)
    static const std::string FEATURE_MODIFIED;

    /// @brief feature has been approved but not changed (i.e. after being reguessed)
    static const std::string FEATURE_APPROVED;
    /// @}

    /// @brief method for getting the attribute in the context of object selection
    virtual std::string getAttributeForSelection(SumoXMLAttr key) const;

    /// @brief get tag assigned to this object in string format
    const std::string &getTagStr() const;

    /// @brief get Tag Property assigned to this object
    const TagProperties &getTagProperty() const;

    /// @brief get FXIcon associated to this AC
    FXIcon* getIcon() const;

    /// @brief function to support debugging
    const std::string getID() const;

    /// @brief get Tag Properties
    static const TagProperties& getTagProperties(SumoXMLTag tag);

    /// @brief get tags of all editable element types
    static std::vector<SumoXMLTag> allowedTags(bool onlyDrawables);

    /// @brief get tags of all editable element types using TagProperty Type (TAGPROPERTY_NETELEMENT, TAGPROPERTY_ADDITIONAL, etc.)
    static std::vector<SumoXMLTag> allowedTagsByCategory(int tagPropertyCategory, bool onlyDrawables);

    /// @brief return the number of attributes of the tag with the most highter number of attributes
    static int getHigherNumberOfAttributes();

    /// @name This functions related with generic parameters has to be implemented in all GNEAttributeCarriers
    /// @{

    /// @brief return generic parameters in string format
    virtual std::string getGenericParametersStr() const = 0;

    /// @brief return generic parameters as vector of pairs format
    virtual std::vector<std::pair<std::string, std::string> > getGenericParameters() const = 0;

    /// @brief set generic parameters in string format
    virtual void setGenericParametersStr(const std::string& value) = 0;

    /// @}

    /// @brief check if given string can be parsed to a map/list of generic parameters
    static bool isGenericParametersValid(const std::string& value);

    /// @brief true if a value of type T can be parsed from string
    template<typename T>
    static bool canParse(const std::string& string) {
        try {
            parse<T>(string);
        } catch (NumberFormatException&) {
            return false;
        } catch (EmptyData&) {
            return false;
        } catch (BoolFormatException&) {
            return false;
        }
        return true;
    }

    /// @brief parses a value of type T from string (used for basic types: int, double, bool, etc.)
    template<typename T>
    static T parse(const std::string& string);

    /// @brief true if a value of type T can be parsed from string
    template<typename T>
    static bool canParse(GNENet* net, const std::string& value, bool report) {
        try {
            parse<T>(net, value);
        } catch (FormatException& exception) {
            if (report) {
                WRITE_WARNING(exception.what())
            }
            return false;
        }
        return true;
    }

    /// @brief parses a complex value of type T from string (use for list of edges, list of lanes, etc.)
    template<typename T>
    static T parse(GNENet* net, const std::string& value);

    /// @brief parses a list of specific Attribute Carriers into a string of IDs
    template<typename T>
    static std::string parseIDs(const std::vector<T>& ACs);

    /// @brief check if lanes are consecutives
    static bool lanesConsecutives(const std::vector<GNELane*>& lanes);

    /// @brief Parse attribute from XML and show warnings if there are problems parsing it
    template <typename T>
    static T parseAttributeFromXML(const SUMOSAXAttributes& attrs, const std::string& objectID, const SumoXMLTag tag, const SumoXMLAttr attribute, bool& abort) {
        bool parsedOk = true;
        // @brief declare string values
        std::string defaultValue, parsedAttribute, additionalOfWarningMessage;
        // obtain tag properties
        const auto& tagProperties = getTagProperties(tag);
        // first check if attribute is deprecated
        if (tagProperties.isAttributeDeprecated(attribute)) {
            // show warning if deprecateda ttribute is in the SUMOSAXAttributes
            if (attrs.hasAttribute(attribute)) {
                WRITE_WARNING("Attribute " + toString(attribute) + "' of " + tagProperties.getTagStr() + " is deprecated and will not be loaded.");
            }
            // return a dummy value
            return parse<T>("");
        }
        // obtain attribute properties (Only for improving efficiency)
        const auto& attrProperties = tagProperties.getAttributeProperties(attribute);
        // check if we're obtaining attribute of an object with an already parsed ID
        if (objectID != "") {
            additionalOfWarningMessage = tagProperties.getTagStr() + " with ID '" + objectID + "'";
        } else {
            additionalOfWarningMessage = tagProperties.getTagStr();
        }
        // set a special default value for numerical and boolean attributes (To avoid errors parsing)
        if (attrProperties.isNumerical() || attrProperties.isBool()) {
            defaultValue = "0";
        } else if (attrProperties.isColor()) {
            defaultValue = "black";
        } else if (attrProperties.isposition()) {
            defaultValue = "0,0";
        }
        // first check that attribute exists in XML
        if (attrs.hasAttribute(attribute)) {
            // First check if attribute can be parsed to string
            parsedAttribute = attrs.get<std::string>(attribute, objectID.c_str(), parsedOk, false);
            // check that sucesfully parsed attribute can be converted to type T
            if (parsedOk && !canParse<T>(parsedAttribute)) {
                parsedOk = false;
            }
            // declare a string for details about error formats
            std::string errorFormat;
            // set extra check for ID Values
            if (attribute == SUMO_ATTR_ID) {
                if (parsedAttribute.empty()) {
                    errorFormat = "ID cannot be empty; ";
                    parsedOk = false;
                } else if (tagProperties.isDetector()) {
                    // special case for detectors (because in this case empty spaces are allowed)
                    if (SUMOXMLDefinitions::isValidDetectorID(parsedAttribute) == false) {
                        errorFormat = "Detector ID contains invalid characters; ";
                        parsedOk = false;
                    }
                } else if (SUMOXMLDefinitions::isValidNetID(parsedAttribute) == false) {
                    errorFormat = "ID contains invalid characters; ";
                    parsedOk = false;
                }
            }
            // Set extra checks for int values
            if (attrProperties.isInt()) {
                if (canParse<int>(parsedAttribute)) {
                    // obtain int value
                    int parsedIntAttribute = parse<int>(parsedAttribute);
                    // check if attribute can be negative or zero
                    if (attrProperties.isPositive() && (parsedIntAttribute < 0)) {
                        errorFormat = "Cannot be negative; ";
                        parsedOk = false;
                    } else if (attrProperties.cannotBeZero() && (parsedIntAttribute == 0)) {
                        errorFormat = "Cannot be zero; ";
                        parsedOk = false;
                    }
                } else if (canParse<double>(parsedAttribute)) {
                    errorFormat = "Float cannot be reinterpreted as int; ";
                    parsedOk = false;
                } else {
                    errorFormat = "Cannot be parsed to int; ";
                    parsedOk = false;
                }
            }
            // Set extra checks for float(double) values
            if (attrProperties.isFloat()) {
                if (canParse<double>(parsedAttribute)) {
                    // obtain double value
                    double parsedDoubleAttribute = parse<double>(parsedAttribute);
                    //check if can be negative and Zero
                    if (attrProperties.isPositive() && (parsedDoubleAttribute < 0)) {
                        errorFormat = "Cannot be negative; ";
                        parsedOk = false;
                    } else if (attrProperties.cannotBeZero() && (parsedDoubleAttribute == 0)) {
                        errorFormat = "Cannot be zero; ";
                        parsedOk = false;
                    }
                } else {
                    errorFormat = "Cannot be parsed to float; ";
                    parsedOk = false;
                }
            }
            // Set extra checks for position values
            if (attrProperties.isposition()) {
                // check if we're parsing a single position or an entire shape
                if (attrProperties.isList()) {
                    // check if parsed attribute can be parsed to Position Vector
                    if(!canParse<PositionVector>(parsedAttribute)) {
                        errorFormat = "List of Positions aren't neither x,y nor x,y,z; ";
                        parsedOk = false;
                    }
                } else if (!canParse<Position>(parsedAttribute)) {
                    errorFormat = "Position is neither x,y nor x,y,z; ";
                    parsedOk = false;
                }
            }
            // set extra check for time(double) values
            if (attrProperties.isTime()) {
                if (canParse<double>(parsedAttribute)) {
                    // parse to SUMO Real and check if is negative
                    if (parse<double>(parsedAttribute) < 0) {
                        errorFormat = "Time cannot be negative; ";
                        parsedOk = false;
                    }
                } else {
                    errorFormat = "Cannot be parsed to time; ";
                    parsedOk = false;
                }
            }
            // set extra check for probability values
            if (attrProperties.isProbability()) {
                if (canParse<double>(parsedAttribute)) {
                    // parse to double and check if is between [0,1]
                    double probability = parse<double>(parsedAttribute);
                    if (probability < 0) {
                        errorFormat = "Probability cannot be smaller than 0; ";
                        parsedOk = false;
                    } else if (probability > 1) {
                        errorFormat = "Probability cannot be greather than 1; ";
                        parsedOk = false;
                    }
                } else {
                    errorFormat = "Cannot be parsed to probability; ";
                    parsedOk = false;
                }
            }
            // set extra check for range values
            if (attrProperties.hasAttrRange()) {
                if (canParse<double>(parsedAttribute)) {
                    // parse to double and check if is in range
                    double range = parse<double>(parsedAttribute);
                    if (range < attrProperties.getMinimumRange()) {
                        errorFormat = "Float cannot be smaller than " + toString(attrProperties.getMinimumRange())+ "; ";
                        parsedOk = false;
                    } else if (range > attrProperties.getMaximumRange()) {
                        errorFormat = "Float cannot be greather than " + toString(attrProperties.getMaximumRange())+ "; ";
                        parsedOk = false;
                    }
                } else {
                    errorFormat = "Cannot be parsed to float; ";
                    parsedOk = false;
                }
            }
            // set extra check for discrete values
            if (attrProperties.isDiscrete()) {
                // search value in the list of discretes values of attribute properties
                auto finder = std::find(attrProperties.getDiscreteValues().begin(), attrProperties.getDiscreteValues().end(), parsedAttribute);
                // check if attribute is valid
                if (finder == attrProperties.getDiscreteValues().end()) {
                    errorFormat = "value is not within the set of allowed values for attribute '" + toString(attribute) + "'";
                    parsedOk = false;
                }
            }
            // set extra check for color values
            if (attrProperties.isColor() && !canParse<RGBColor>(parsedAttribute)) {
                errorFormat = "Invalid RGB format or named color; ";
                parsedOk = false;
            }
            // set extra check for filename values
            if (attrProperties.isFilename()) {
                if (SUMOXMLDefinitions::isValidFilename(parsedAttribute) == false) {
                    errorFormat = "Filename contains invalid characters; ";
                    parsedOk = false;
                } else if (parsedAttribute.empty()) {
                    errorFormat = "Filename cannot be empty; ";
                    parsedOk = false;
                }
            }
            // set extra check for name values
            if ((attribute == SUMO_ATTR_NAME) && !SUMOXMLDefinitions::isValidAttribute(parsedAttribute)) {
                errorFormat = "name contains invalid characters; ";
                parsedOk = false;
            }
            // set extra check for SVCPermissions values
            if (attrProperties.isVClass()) {
                if (!canParseVehicleClasses(parsedAttribute)) {
                    errorFormat = "List of VClasses isn't valid; ";
                    parsedAttribute = defaultValue;
                    parsedOk = false;
                }
            }
            // set extra check for Vehicle Classes
            if ((!parsedOk) && (attribute == SUMO_ATTR_VCLASS)) {
                errorFormat = "Is not a part of defined set of Vehicle Classes; ";
            }
            // set extra check for Vehicle Classes
            if ((!parsedOk) && (attribute == SUMO_ATTR_GUISHAPE)) {
                errorFormat = "Is not a part of defined set of Gui Vehicle Shapes; ";
            }
            // set extra check for RouteProbes
            if ((attribute == SUMO_ATTR_ROUTEPROBE) && !SUMOXMLDefinitions::isValidNetID(parsedAttribute)) {
                errorFormat = "RouteProbe ID contains invalid characters; ";
                parsedOk = false;
            }
            // set extra check for list of edges
            if ((attribute == SUMO_ATTR_EDGES) && parsedAttribute.empty()) {
                errorFormat = "List of edges cannot be empty; ";
                parsedOk = false;
            }
            // set extra check for list of lanes
            if ((attribute == SUMO_ATTR_LANES) && parsedAttribute.empty()) {
                errorFormat = "List of lanes cannot be empty; ";
                parsedOk = false;
            }
            // set extra check for list of VTypes
            if ((attribute == SUMO_ATTR_VTYPES) && !parsedAttribute.empty() && !SUMOXMLDefinitions::isValidListOfTypeID(parsedAttribute)) {
                errorFormat = "List of vTypes contains invalid characters; ";
                parsedOk = false;
            }
            // set extra check for list of RouteProbe
            if ((attribute == SUMO_ATTR_ROUTEPROBE) && !parsedAttribute.empty() && !SUMOXMLDefinitions::isValidNetID(parsedAttribute)) {
                errorFormat = "RouteProbe ID contains invalid characters; ";
                parsedOk = false;
            }
            // If attribute has an invalid format
            if (!parsedOk) {
                // if attribute is optional and has a default value, obtain it as string. In other case, abort.
                if (attrProperties.isOptional() && attrProperties.hasDefaultValue()) {
                    parsedAttribute = attrProperties.getDefaultValue();
                } else {
                    WRITE_WARNING("Format of essential " + attrProperties.getDescription() + " attribute '" + toString(attribute) + "' of " +
                                  additionalOfWarningMessage +  " is invalid; " + errorFormat + tagProperties.getTagStr() + " cannot be created");
                    // abort parsing (and creation) of element
                    abort = true;
                    // set default value (To avoid errors in parse<T>(parsedAttribute))
                    parsedAttribute = defaultValue;
                }
            }
        } else if (tagProperties.canMaskXYZPositions() && (attribute == SUMO_ATTR_POSITION)) {
            // if element can mask their XYPosition, then must be extracted X Y coordiantes separeted
            std::string x, y, z;
            // give a default value to parsedAttribute to avoid problem parsing invalid positions
            parsedAttribute="0,0";
            if(attrs.hasAttribute(SUMO_ATTR_X)) {
                x = attrs.get<std::string>(SUMO_ATTR_X, objectID.c_str(), parsedOk, false);
                // check that X attribute is valid
                if(!canParse<double>(x)) {
                    WRITE_WARNING("Format of essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_X) + "' of " +
                                  additionalOfWarningMessage +  " is invalid; Cannot be parsed to float; " + tagProperties.getTagStr() + " cannot be created");
                    // abort parsing (and creation) of element
                    abort = true;
                }
            } else {
                WRITE_WARNING("Essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_X) + "' of " +
                              additionalOfWarningMessage +  " is missing; " + tagProperties.getTagStr() + " cannot be created");
                // abort parsing (and creation) of element
                abort = true;
            }
            if(attrs.hasAttribute(SUMO_ATTR_Y)) {
                y = attrs.get<std::string>(SUMO_ATTR_Y, objectID.c_str(), parsedOk, false);
                // check that X attribute is valid
                if(!canParse<double>(y)) {
                    WRITE_WARNING("Format of essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_Y) + "' of " +
                                  additionalOfWarningMessage + " is invalid; Cannot be parsed to float; " + tagProperties.getTagStr() + " cannot be created");
                    // abort parsing (and creation) of element
                    abort = true;
                }
            } else {
                WRITE_WARNING("Essential " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_Y) + "' of " +
                            additionalOfWarningMessage +  " is missing; " + tagProperties.getTagStr() + " cannot be created");
                // abort parsing (and creation) of element
                abort = true;
            }
            // Z attribute is optional
            if(attrs.hasAttribute(SUMO_ATTR_Z)) {                
                z = attrs.get<std::string>(SUMO_ATTR_Z, objectID.c_str(), parsedOk, false);
                // check that Z attribute is valid
                if(!canParse<double>(z)) {
                    WRITE_WARNING("Format of optional " + attrProperties.getDescription() + " attribute '" + toString(SUMO_ATTR_Z) + "' of " +
                                  additionalOfWarningMessage + " is invalid; Cannot be parsed to float; " + tagProperties.getTagStr() + " cannot be created");
                    // leave Z attribute empty
                    z.clear();
                }
            }
            // create Position attribute using parsed coordinates X, Y and, optionally, Z
            if(!abort) {
                if(z.empty()) {
                    parsedAttribute = x + "," + y;
                } else {
                    parsedAttribute = x + "," + y + "," + z;
                }
            }
        } else {
            // if attribute is optional and has a default value, obtain it. In other case, abort.
            if (attrProperties.isOptional() && attrProperties.hasDefaultValue()) {
                parsedAttribute = attrProperties.getDefaultValue();
            } else {
                WRITE_WARNING("Essential " + attrProperties.getDescription() + " attribute '" + toString(attribute) + "' of " +
                              additionalOfWarningMessage +  " is missing; " + tagProperties.getTagStr() + " cannot be created");
                // abort parsing (and creation) of element
                abort = true;
                // set default value (To avoid errors in parse<T>(parsedAttribute))
                parsedAttribute = defaultValue;
            }
        }
        // return parsed attribute
        return parse<T>(parsedAttribute);
    }

    /// @brief function to calculate circle resolution for all circles drawn in drawGL(...) functions
    static int getCircleResolution(const GUIVisualizationSettings& settings);

protected:
    /// @brief method for setting the attribute and nothing else (used in GNEChange_Attribute)
    virtual void setAttribute(SumoXMLAttr key, const std::string& value) = 0;

    /// @brief method for check if mouse is over objects
    virtual void mouseOverObject(const GUIVisualizationSettings& s) const = 0;

    /// @brief the xml tag to which this attribute carrier corresponds
    const TagProperties &myTagProperty;

    /// @brief boolean to check if this AC is selected (instead of GUIGlObjectStorage)
    bool mySelected;

    /// @brief dummy TagProperty used for reference some elements (for Example, dummyEdge)
    static TagProperties dummyTagProperty;

private:
    /// @brief fill Attribute Carriers
    static void fillAttributeCarriers();

    /// @brief map with the tags properties
    static std::map<SumoXMLTag, TagProperties> myTagProperties;

    /// @brief Invalidated copy constructor.
    GNEAttributeCarrier(const GNEAttributeCarrier&) = delete;

    /// @brief Invalidated assignment operator
    GNEAttributeCarrier& operator=(const GNEAttributeCarrier& src) = delete;
};

#endif

/****************************************************************************/

