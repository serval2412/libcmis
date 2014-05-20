#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>

#include <string>

#define private public
#define protected public

#include <mockup-config.h>

#include "onedrive-session.hxx"
#include "oauth2-handler.hxx"
#include "document.hxx"

using namespace std;
using namespace libcmis;

static const string CLIENT_ID ( "mock-id" );
static const string CLIENT_SECRET ( "mock-secret" );
static const string USERNAME( "mock-user" );
static const string PASSWORD( "mock-password" );
static const string LOGIN_URL ("https://login/url" );
static const string APPROVAL_URL ("https://approval/url" );
static const string AUTH_URL ( "https://auth/url" );
static const string TOKEN_URL ( "https://token/url" );
static const string SCOPE ( "https://scope/url" );
static const string REDIRECT_URI ("redirect:uri" );
static const string BASE_URL ( "https://base/url" );

class OneDriveTest : public CppUnit::TestFixture
{
    public:
        void sessionAuthenticationTest( );
        void sessionExpiryTokenGetTest( );
        void sessionExpiryTokenPostTest( );
        void sessionExpiryTokenPutTest( );
        void sessionExpiryTokenDeleteTest( );
        void getRepositoriesTest( );

        CPPUNIT_TEST_SUITE( OneDriveTest );
        CPPUNIT_TEST( sessionAuthenticationTest );
        CPPUNIT_TEST( sessionExpiryTokenGetTest );
        //CPPUNIT_TEST( sessionExpiryTokenPutTest );
        //CPPUNIT_TEST( sessionExpiryTokenPostTest );
        //CPPUNIT_TEST( sessionExpiryTokenDeleteTest );
        //CPPUNIT_TEST( getRepositoriesTest );
        CPPUNIT_TEST_SUITE_END( );

    private:
        OneDriveSession getTestSession( string username, string password );
};

OneDriveSession OneDriveTest::getTestSession( string username, string password )
{
    libcmis::OAuth2DataPtr oauth2(
        new libcmis::OAuth2Data( AUTH_URL, TOKEN_URL, SCOPE,
                                 REDIRECT_URI, CLIENT_ID, CLIENT_SECRET ));
    curl_mockup_reset( );
    string empty;
    // login, authentication & approval are done manually at the moment, so I'll
    // temporarily borrow them from gdrive
    //login response
    string loginIdentifier = string("scope=") + SCOPE +
                             string("&redirect_uri=") + REDIRECT_URI +
                             string("&response_type=code") +
                             string("&client_id=") + CLIENT_ID;
    curl_mockup_addResponse ( AUTH_URL.c_str(), loginIdentifier.c_str( ),
                            "GET", DATA_DIR "/gdrive/login.html", 200, true);

    //authentication response
    curl_mockup_addResponse( LOGIN_URL.c_str( ), empty.c_str( ), "POST",
                             DATA_DIR "/gdrive/approve.html", 200, true);

    //approval response
    curl_mockup_addResponse( APPROVAL_URL.c_str( ), empty.c_str( ),
                             "POST", DATA_DIR "/gdrive/authcode.html", 200, true);


    // token response
    curl_mockup_addResponse ( TOKEN_URL.c_str( ), empty.c_str( ), "POST",
                              DATA_DIR "/onedrive/token-response.json", 200, true );

    return OneDriveSession( BASE_URL, username, password, oauth2, false );
}

void OneDriveTest::sessionAuthenticationTest( )
{
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );
    string empty;

    // Check token request
    string expectedTokenRequest =
        string( "code=AuthCode") +
        string( "&client_id=") + CLIENT_ID +
        string( "&client_secret=") + CLIENT_SECRET +
        string( "&redirect_uri=") + REDIRECT_URI +
        string( "&grant_type=authorization_code" );

    string tokenRequest( curl_mockup_getRequestBody( TOKEN_URL.c_str(), empty.c_str( ),
                                                 "POST" ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong token request",
                                  expectedTokenRequest, tokenRequest );

    // Check token
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong access token",
         string ( "mock-access-token" ),
         session.m_oauth2Handler->getAccessToken( ));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong refresh token",
        string ("mock-refresh-token"),
        session.m_oauth2Handler->getRefreshToken( ));
}

void OneDriveTest::sessionExpiryTokenGetTest( )
{
    // Access_token will expire after expires_in seconds,
    // We need to use the refresh key to get a new one.

    curl_mockup_reset( );
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );

    curl_mockup_reset( );
    static const string objectId("aFileId");
    string url = BASE_URL + "/me/skydrive/files/" + objectId;

    // 401 response, token is expired
    curl_mockup_addResponse( url.c_str( ),"", "GET", "", 401, false );

    curl_mockup_addResponse( TOKEN_URL.c_str(), "",
                             "POST", DATA_DIR "/onedrive/refresh-response.json", 200, true);
    try
    {
        // GET expires, need to refresh then GET again
        libcmis::ObjectPtr obj = session.getObject( objectId );
    }
    catch ( ... )
    {
        if ( session.getHttpStatus( ) == 401 )
        {
            // Check if access token is refreshed
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                   "wrong access token",
                   string ( "new-access-token" ),
                   session.m_oauth2Handler->getAccessToken( ) );
        }
    }
}

void OneDriveTest::sessionExpiryTokenPostTest( )
{
    // Access_token will expire after expires_in seconds,
    // We need to use the refresh key to get a new one.

    curl_mockup_reset( );
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );

    curl_mockup_reset( );
    static const string folderId("aFileId");
    const string folderUrl = BASE_URL + "me/skydrive/files/" + folderId;
    const string metaUrl = BASE_URL + "/files";

    curl_mockup_addResponse( TOKEN_URL.c_str(), "",
                             "POST", DATA_DIR "/onedrive/refresh_response.json", 200, true);

    curl_mockup_addResponse( folderUrl.c_str( ), "",
                               "GET", DATA_DIR "/onedrive/folder.json", 200, true );

    // 401 response, token is expired
    // refresh and then POST again
    curl_mockup_addResponse( metaUrl.c_str( ), "",
                               "POST", "", 401, false );
    libcmis::FolderPtr parent = session.getFolder( folderId );

    try
    {
        PropertyPtrMap properties;
        // POST expires, need to refresh then POST again
        parent->createFolder( properties );
    }
    catch ( ... )
    {
        if ( session.getHttpStatus( ) == 401 )
        {
            // Check if access token is refreshed
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                   "wrong access token",
                   string ( "new-access-token" ),
                   session.m_oauth2Handler->getAccessToken( ) );
        }
    }
}

void OneDriveTest::sessionExpiryTokenDeleteTest( )
{
    // Access_token will expire after expires_in seconds,
    // We need to use the refresh key to get a new one.

    curl_mockup_reset( );
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );

    curl_mockup_reset( );
    static const string objectId("aFileId");
    string url = BASE_URL + "/me/skydrive/files/" + objectId;

    curl_mockup_addResponse( url.c_str( ), "",
                               "GET", DATA_DIR "/onedrive/document2.json", 200, true);

    curl_mockup_addResponse( TOKEN_URL.c_str(), "",
                             "POST", DATA_DIR "/onedrive/refresh_response.json", 200, true);
    // 401 response, token is expired
    curl_mockup_addResponse( url.c_str( ),"", "DELETE", "", 401, false);

    libcmis::ObjectPtr obj = session.getObject( objectId );

    libcmis::ObjectPtr object = session.getObject( objectId );

    try
    {
        // DELETE expires, need to refresh then DELETE again
        object->remove( );
    }
    catch ( ... )
    {
        if ( session.getHttpStatus( ) == 401 )
        {
            // Check if access token is refreshed
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                   "wrong access token",
                   string ( "new-access-token" ),
                   session.m_oauth2Handler->getAccessToken( ) );
            const struct HttpRequest* deleteRequest = curl_mockup_getRequest( url.c_str( ), "", "DELETE" );
            CPPUNIT_ASSERT_MESSAGE( "Delete request not sent", deleteRequest );
        }
    }

}

void OneDriveTest::sessionExpiryTokenPutTest( )
{
    // Access_token will expire after expires_in seconds,
    // We need to use the refresh key to get a new one.

    curl_mockup_reset( );
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );

    curl_mockup_reset( );
    static const string objectId("aFileId");
    string url = BASE_URL + "/me/skydrive/files/" + objectId;

    curl_mockup_addResponse( TOKEN_URL.c_str(), "",
                             "POST", DATA_DIR "/onedrive/refresh_response.json", 200, true);

    curl_mockup_addResponse( url.c_str( ), "",
                               "GET", DATA_DIR "/onedrive/document.json", 200, true );

    // 401 response, token is expired
    curl_mockup_addResponse( url.c_str( ),"", "PUT", "", 401, false );

    libcmis::ObjectPtr object = session.getObject( objectId );

    try
    {
        // PUT expires, need to refresh then PUT again
        object->updateProperties( object->getProperties( ) );
    }
    catch ( ... )
    {
        if ( session.getHttpStatus( ) == 401 )
        {
            // Check if access token is refreshed
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                   "wrong access token",
                   string ( "new-access-token" ),
                   session.m_oauth2Handler->getAccessToken( ) );
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( OneDriveTest );