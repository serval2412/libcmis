/* libcmis
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 SUSE <cbosdonnat@suse.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#include <sstream>

#include "atom-object-type.hxx"
#include "xml-utils.hxx"

using namespace std;
using namespace boost;


AtomObjectType::AtomObjectType( AtomPubSession* session, string id ) throw ( libcmis::Exception ) :
    m_session( session ),
    m_refreshTimestamp( 0 ),
    m_selfUrl( ),
    m_id( id ),
    m_localName( ),
    m_localNamespace( ),
    m_displayName( ),
    m_queryName( ),
    m_description( ),
    m_parentTypeId( ),
    m_baseTypeId( ),
    m_childrenUrl( ),
    m_creatable( false ),
    m_fileable( false ),
    m_queryable( false ),
    m_fulltextIndexed( false ),
    m_includedInSupertypeQuery( false ),
    m_controllablePolicy( false ),
    m_controllableAcl( false ),
    m_versionable( false ),
    m_contentStreamAllowed( libcmis::ObjectType::Allowed ),
    m_propertiesTypes( )
{
    refresh( );
}

AtomObjectType::AtomObjectType( AtomPubSession* session, xmlNodePtr entryNd ) throw ( libcmis::Exception ) :
    m_session( session ),
    m_refreshTimestamp( 0 ),
    m_selfUrl( ),
    m_id( ),
    m_localName( ),
    m_localNamespace( ),
    m_displayName( ),
    m_queryName( ),
    m_description( ),
    m_parentTypeId( ),
    m_baseTypeId( ),
    m_childrenUrl( ),
    m_creatable( false ),
    m_fileable( false ),
    m_queryable( false ),
    m_fulltextIndexed( false ),
    m_includedInSupertypeQuery( false ),
    m_controllablePolicy( false ),
    m_controllableAcl( false ),
    m_versionable( false ),
    m_contentStreamAllowed( libcmis::ObjectType::Allowed ),
    m_propertiesTypes( )
{
    xmlDocPtr doc = libcmis::wrapInDoc( entryNd );
    refreshImpl( doc );
    xmlFreeDoc( doc );
}

AtomObjectType::AtomObjectType( const AtomObjectType& copy ) :
    m_session( copy.m_session ),
    m_refreshTimestamp( copy.m_refreshTimestamp ),
    m_selfUrl( copy.m_selfUrl ),
    m_id( copy.m_id ),
    m_localName( copy.m_localName ),
    m_localNamespace( copy.m_localNamespace ),
    m_displayName( copy.m_displayName ),
    m_queryName( copy.m_queryName ),
    m_description( copy.m_description ),
    m_parentTypeId( copy.m_parentTypeId ),
    m_baseTypeId( copy.m_baseTypeId ),
    m_childrenUrl( copy.m_childrenUrl ),
    m_creatable( copy.m_creatable ),
    m_fileable( copy.m_fileable ),
    m_queryable( copy.m_queryable ),
    m_fulltextIndexed( copy.m_fulltextIndexed ),
    m_includedInSupertypeQuery( copy.m_includedInSupertypeQuery ),
    m_controllablePolicy( copy.m_controllablePolicy ),
    m_controllableAcl( copy.m_controllableAcl ),
    m_versionable( copy.m_versionable ),
    m_contentStreamAllowed( copy.m_contentStreamAllowed ),
    m_propertiesTypes( copy.m_propertiesTypes )
{
}

AtomObjectType::~AtomObjectType( )
{
}

AtomObjectType& AtomObjectType::operator=( const AtomObjectType& copy )
{
    if ( this != &copy )
    {
        m_session = copy.m_session;
        m_refreshTimestamp = copy.m_refreshTimestamp;
        m_selfUrl = copy.m_selfUrl;
        m_id = copy.m_id;
        m_localName = copy.m_localName;
        m_localNamespace = copy.m_localNamespace;
        m_displayName = copy.m_displayName;
        m_queryName = copy.m_queryName;
        m_description = copy.m_description;
        m_parentTypeId = copy.m_parentTypeId;
        m_baseTypeId = copy.m_baseTypeId;
        m_childrenUrl = copy.m_childrenUrl;
        m_creatable = copy.m_creatable;
        m_fileable = copy.m_fileable;
        m_queryable = copy.m_queryable;
        m_fulltextIndexed = copy.m_fulltextIndexed;
        m_includedInSupertypeQuery = copy.m_includedInSupertypeQuery;
        m_controllablePolicy = copy.m_controllablePolicy;
        m_controllableAcl = copy.m_controllableAcl;
        m_versionable = copy.m_versionable;
        m_contentStreamAllowed = copy.m_contentStreamAllowed;
        m_propertiesTypes = copy.m_propertiesTypes;
    }

    return *this;
}

libcmis::ObjectTypePtr AtomObjectType::getParentType( ) throw ( libcmis::Exception )
{
    return m_session->getType( m_parentTypeId );
}

libcmis::ObjectTypePtr AtomObjectType::getBaseType( ) throw ( libcmis::Exception )
{
    return m_session->getType( m_baseTypeId );
}

vector< libcmis::ObjectTypePtr > AtomObjectType::getChildren( ) throw ( libcmis::Exception )
{
    vector< libcmis::ObjectTypePtr > children;
    string buf;
    try
    {
        buf = m_session->httpGetRequest( m_childrenUrl )->getStream( )->str( );
    }
    catch ( const CurlException& e )
    {
        throw e.getCmisException( );
    }

    xmlDocPtr doc = xmlReadMemory( buf.c_str(), buf.size(), m_childrenUrl.c_str(), NULL, 0 );
    if ( NULL != doc )
    {
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext( doc );
        libcmis::registerNamespaces( xpathCtx );
        if ( NULL != xpathCtx )
        {
            const string& entriesReq( "//atom:entry" );
            xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression( BAD_CAST( entriesReq.c_str() ), xpathCtx );

            if ( NULL != xpathObj && NULL != xpathObj->nodesetval )
            {
                int size = xpathObj->nodesetval->nodeNr;
                for ( int i = 0; i < size; i++ )
                {
                    xmlNodePtr node = xpathObj->nodesetval->nodeTab[i];
                    libcmis::ObjectTypePtr type( new AtomObjectType( m_session, node ) );
                    children.push_back( type );
                }
            }

            xmlXPathFreeObject( xpathObj );
        }

        xmlXPathFreeContext( xpathCtx );
    }
    else
    {
        throw new libcmis::Exception( "Failed to parse type children infos" );
    }
    xmlFreeDoc( doc );

    return children;
}

string AtomObjectType::toString( )
{
    stringstream buf;
    
    buf << "Type Description:" << endl << endl;
    buf << "Id: " << getId( ) << endl;
    buf << "Display name: " << getDisplayName( ) << endl;

    buf << "Parent type: " << m_parentTypeId << endl;
    buf << "Base type: " << m_baseTypeId << endl;
    buf << "Children types [(id) Name]: " << endl;
    vector< libcmis::ObjectTypePtr > children = getChildren( );
    for ( vector< libcmis::ObjectTypePtr >::iterator it = children.begin(); it != children.end(); ++it )
    {
        libcmis::ObjectTypePtr type = *it;
        buf << "    (" << type->getId( ) << ")\t" << type->getDisplayName( ) << endl;
    } 

    buf << "Creatable: " << isCreatable( ) << endl;
    buf << "Fileable: " << isFileable( ) << endl;
    buf << "Queryable: " << isQueryable( ) << endl;
    buf << "Full text indexed: " << isFulltextIndexed( ) << endl;
    buf << "Included in supertype query: " << isIncludedInSupertypeQuery( ) << endl;
    buf << "Controllable policy: " << isControllablePolicy( ) << endl;
    buf << "Controllable ACL: " << isControllableACL( ) << endl;

    buf << "Property Definitions [RO/RW (id) Name]: " << endl;
    map< string, libcmis::PropertyTypePtr > propsTypes = getPropertiesTypes( );
    for ( map< string, libcmis::PropertyTypePtr >::iterator it = propsTypes.begin( ); it != propsTypes.end( ); ++it )
    {
        libcmis::PropertyTypePtr propType = it->second;
        string updatable( "RO" );
        if ( propType->isUpdatable( ) )
            updatable = string( "RW" );

        buf << "    " << updatable << "\t (" << propType->getId( ) << ")\t" 
            << propType->getDisplayName( ) << endl;
    }

    return buf.str();
}

void AtomObjectType::refreshImpl( xmlDocPtr doc ) throw ( libcmis::Exception )
{
    bool createdDoc = ( NULL == doc );
    if ( createdDoc )
    {
        string pattern = m_session->getAtomRepository()->getUriTemplate( UriTemplate::TypeById );
        map< string, string > vars;
        vars[URI_TEMPLATE_VAR_ID] = getId( );
        string url = m_session->createUrl( pattern, vars );

        string buf;
        try
        {
            buf  = m_session->httpGetRequest( url )->getStream()->str();
        }
        catch ( const CurlException& e )
        {
            if ( ( e.getErrorCode( ) == CURLE_HTTP_RETURNED_ERROR ) &&
                 ( string::npos != e.getErrorMessage( ).find( "404" ) ) )
            {
                string msg = "No such type: ";
                msg += getId( );
                throw libcmis::Exception( msg );
            }
            else
                throw e.getCmisException( );
        }

        doc = xmlReadMemory( buf.c_str(), buf.size(), m_selfUrl.c_str(), NULL, 0 );

        if ( NULL == doc )
            throw libcmis::Exception( "Failed to parse object infos" );
    }

    extractInfos( doc );
    
    if ( createdDoc )
        xmlFreeDoc( doc );
}

void AtomObjectType::extractInfos( xmlDocPtr doc ) throw ( libcmis::Exception )
{
    xmlXPathContextPtr xpathCtx = xmlXPathNewContext( doc );

    // Register the Service Document namespaces
    libcmis::registerNamespaces( xpathCtx );

    if ( NULL != xpathCtx )
    {
        // Get the self URL
        string selfUrlReq( "//atom:link[@rel='self']/attribute::href" );
        m_selfUrl = libcmis::getXPathValue( xpathCtx, selfUrlReq );
        
        // Get the children URL
        string childrenUrlReq( "//atom:link[@rel='down' and @type='application/atom+xml;type=feed']/attribute::href" );
        m_childrenUrl = libcmis::getXPathValue( xpathCtx, childrenUrlReq );
        
        // Get the cmisra:type node
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression( BAD_CAST( "//cmisra:type" ), xpathCtx );
        if ( NULL != xpathObj && NULL != xpathObj->nodesetval && xpathObj->nodesetval->nodeNr )
        {
            xmlNodePtr typeNode = xpathObj->nodesetval->nodeTab[0];

            for ( xmlNodePtr child = typeNode->children; child; child = child->next )
            {
                xmlChar* content = xmlNodeGetContent( child );
                string value( ( const char * ) content );
                
                if ( xmlStrEqual( child->name, BAD_CAST( "id" ) ) )
                    m_id = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "localName" ) ) )
                    m_localName = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "localNamespace" ) ) )
                    m_localNamespace = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "displayName" ) ) )
                    m_displayName = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "queryName" ) ) )
                    m_queryName = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "description" ) ) )
                    m_description = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "baseId" ) ) )
                    m_baseTypeId = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "parentId" ) ) )
                    m_parentTypeId = value;
                else if ( xmlStrEqual( child->name, BAD_CAST( "creatable" ) ) )
                    m_creatable = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "fileable" ) ) )
                    m_fileable = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "queryable" ) ) )
                    m_queryable = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "fulltextIndexed" ) ) )
                    m_fulltextIndexed = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "includedInSupertypeQuery" ) ) )
                    m_includedInSupertypeQuery = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "controllablePolicy" ) ) )
                    m_controllablePolicy = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "controllableACL" ) ) )
                    m_controllableAcl = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "versionable" ) ) )
                    m_versionable = libcmis::parseBool( value );
                else if ( xmlStrEqual( child->name, BAD_CAST( "contentStreamAllowed" ) ) )
                {
                    libcmis::ObjectType::ContentStreamAllowed streamAllowed = libcmis::ObjectType::Allowed;
                    if ( value == "notallowed" )
                        streamAllowed = libcmis::ObjectType::NotAllowed;
                    else if ( value == "required" )
                        streamAllowed = libcmis::ObjectType::Required;

                    m_contentStreamAllowed = streamAllowed;
                }
                else 
                {
                    libcmis::PropertyTypePtr type( new libcmis::PropertyType( child ) );
                    m_propertiesTypes.insert( pair< string, libcmis::PropertyTypePtr >( type->getId(), type ) );
                }

                xmlFree( content );
            }
        }
        xmlXPathFreeObject( xpathObj );
    }
    xmlXPathFreeContext( xpathCtx );
}
