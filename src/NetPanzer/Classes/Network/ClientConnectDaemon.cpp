/*
Copyright (C) 1998 Pyrosoft Inc. (www.pyrosoftgames.com), Matthew Bogue
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <config.h>
#include "Classes/Network/ClientConnectDaemon.hpp"

#include <stdio.h>

#include "Interfaces/PlayerInterface.hpp"
#include "Units/UnitInterface.hpp"
#include "NetworkClient.hpp"
#include "Interfaces/GameConfig.hpp"
#include "Interfaces/GameManager.hpp"

#include "Util/Log.hpp"
#include "ConnectNetMessage.hpp"
#include "Interfaces/ConsoleInterface.hpp"
#include "Views/Game/LoadingView.hpp"

enum { _connect_state_idle,
       _connect_state_wait_for_connect_start,
       _connect_state_send_connect_request,
       _connect_state_wait_for_connect_result,
       _connect_state_wait_for_server_game_setup,
       _connect_state_setup_client_game,
       _connect_state_connect_failure
     };


Timer ClientConnectDaemon::failure_display_timer;
Timer ClientConnectDaemon::time_out_timer;
int   ClientConnectDaemon::time_out_counter = 0;
unsigned char ClientConnectDaemon::connection_state;

#define _CLIENT_CONNECT_TIME_OUT_TIME (6.0)
#define _CLIENT_CONNECT_RETRY_LIMIT   (5)


void ClientConnectDaemon::startConnectDaemon()
{
    connection_state = _connect_state_idle;
}

void ClientConnectDaemon::shutdownConnectDaemon()
{
    ConnectMesgNetPanzerClientDisconnect client_disconnect;

    client_disconnect.setPlayerID(
            PlayerInterface::getLocalPlayerIndex());

    NetworkClient::sendMessage(&client_disconnect,
            sizeof(ConnectMesgNetPanzerClientDisconnect));
}

void ClientConnectDaemon::startConnectionProcess( )
{
    failure_display_timer.changePeriod( 10 );
    time_out_timer.changePeriod( _CLIENT_CONNECT_TIME_OUT_TIME );
    time_out_counter = 0;
    connection_state = _connect_state_wait_for_connect_start;
}

void ClientConnectDaemon::netMessageLinkAck(const NetMessage* message)
{
    ClientConnectJoinRequestAck *join_request_ack_mesg;
    char buf[80];

    join_request_ack_mesg = (ClientConnectJoinRequestAck *) message;

    switch( join_request_ack_mesg->getResultCode() ) {
    case _join_request_result_success :
        LoadingView::append( "Link to Server Established" );
        sprintf( buf, "Protocol Version: %u",
                join_request_ack_mesg->getServerProtocolVersion());
        LoadingView::append( buf );
        break;

    case _join_request_result_invalid_protocol :
        LoadingView::append( "Link to Server FAILED!" );
        LoadingView::append( "Incorrect Network Protocol Revision" );
        LoadingView::append( "Please get a newer netPanzer version." );
        sprintf( buf, "Server Protocol Version: %u",
                join_request_ack_mesg->getServerProtocolVersion());
        LoadingView::append( buf );
        connection_state = _connect_state_connect_failure;
        failure_display_timer.reset();
        break;

    case _join_request_result_server_busy :
        LoadingView::append( "Link to Server FAILED!" );
        LoadingView::append( "Server is VERY busy" );
        LoadingView::append( "Please try in a few minutes" );
        connection_state = _connect_state_connect_failure;
        failure_display_timer.reset();
        break;

    default:
        LOG ( ("Unknown ACk result?!?") );
        break;
    }

}

void ClientConnectDaemon::netMessageConnectProcessUpdate(const NetMessage* message )
{
    ConnectProcessUpdate *process_update;

    process_update = (ConnectProcessUpdate *) message;

    char buf[80];
    snprintf(buf, sizeof(buf), "Your Position In Queue is %d ",
            process_update->getQueuePosition());

    LoadingView::append( buf );
}

void ClientConnectDaemon::netMessageConnectProcessMessage(const NetMessage* message)
{
    ConnectProcessStateMessage *state_mesg;

    char str_buf[128];

    state_mesg = (ConnectProcessStateMessage *) message;

    switch ( state_mesg->getMessageEnum() ) {
    case _connect_state_message_load_game_data : {
            LoadingView::append( "Loading Game Data ..." );
        }
        break;

    case  _connect_state_message_sync_player_info : {
            LoadingView::append( "Synchronizing Player Info ..." );
        }
        break;

    case _connect_state_message_sync_player_info_percent : {
            snprintf(str_buf, sizeof(str_buf),
                    "Synchronizing Player Info ... (%d%%)",
                    state_mesg->getPercentComplete());
            LoadingView::update( str_buf );
        }
        break;

    case  _connect_state_message_sync_units : {
            LoadingView::append( "Synchronizing Game Elements ..." );
        }
        break;

    case _connect_state_message_sync_units_percent : {
            snprintf(str_buf, sizeof(str_buf),
                    "Synchronizing Game Elements ... (%d%%)",
                    state_mesg->getPercentComplete());
            LoadingView::update( str_buf );
        }
        break;

    case  _connect_state_sync_complete : {
            LoadingView::append( "Game Synchronized" );
            LoadingView::loadFinish();
        }
        break;
    }
}

void ClientConnectDaemon::netMessageConnectServerDisconnect(const NetMessage* )
{
    ConsoleInterface::postMessage(Color::unitAqua, false, 0, "Server has terminated the game" );
}

void ClientConnectDaemon::processNetMessage(const NetMessage* message)
{
    switch (message->message_id) {
    case _net_message_id_connect_join_game_request_ack : {
            netMessageLinkAck( message );
        }
        break;

    case _net_message_id_client_connect_process_update : {
            netMessageConnectProcessUpdate( message );
        }
        break;

    case _net_message_id_client_connect_process_state_mesg : {
            netMessageConnectProcessMessage( message );
        }
        break;

    case _net_message_id_connect_netPanzer_server_disconnect : {
            netMessageConnectServerDisconnect( message );
        }
        break;

    default :
        connectFsm( message );
        break;
    } // ** switch

}

void ClientConnectDaemon::serverConnectionBroken()
{
    ConsoleInterface::postMessage(Color::unitRed, false, 0, "CONNECTION TO SERVER HAS BEEN UNEXPECTEDLY BROKEN." );
}

void ClientConnectDaemon::connectFailureResult( unsigned char result_code )
{
    switch( result_code ) {
    case _connect_result_server_busy : {
            LoadingView::append( "Connect Failure: Server Too Busy" );
        }
        break;

    case _connect_result_server_full : {
            LoadingView::append( "Connect Failure: Server Full" );
        }
        break;
    } // ** switch

}

void ClientConnectDaemon::connectFsm(const NetMessage* message )
{
    bool end_cycle = false;

    do {
        switch ( connection_state ) {
        case _connect_state_idle :
            return;

        case _connect_state_wait_for_connect_start : {
                if ( message != 0 ) {
                    if ( message->message_id == _net_message_id_client_start_connect ) {
                        LoadingView::append( "Connecting ..." );
                        connection_state = _connect_state_send_connect_request;
                    } else {
                        end_cycle = true;
                    }
                } // ** if ( message != 0 )
                else {
                    end_cycle = true;
                }

            }
            break;

        case _connect_state_send_connect_request : {
                ClientConnectRequest connect_request;

                NetworkClient::sendMessage(&connect_request, sizeof(ClientConnectRequest));

                connection_state = _connect_state_wait_for_connect_result;

                end_cycle = true;
            }
            break;

        case _connect_state_wait_for_connect_result : {
                if ( message != 0 ) {
                    if ( message->message_id == _net_message_id_client_connect_result ) {
                        ClientConnectResult *connect_result;

                        connect_result = (ClientConnectResult *) message;

                        if ( connect_result->result_code != _connect_result_success ) {
                            connectFailureResult( connect_result->result_code );

                            connection_state = _connect_state_connect_failure;
                            failure_display_timer.reset();
                        } else {
                            ConnectClientSettings client_setting;

                            client_setting.set( 
                                    gameconfig->playername.c_str(),
                                                gameconfig->getUnitColor(),
                                                gameconfig->playerflag.c_str() );

                            NetworkClient::sendMessage( &client_setting, sizeof(ConnectClientSettings));

                            connection_state = _connect_state_wait_for_server_game_setup;
                        }

                    } else {
                        if ( time_out_timer.count() ) {
                            if ( time_out_counter < _CLIENT_CONNECT_RETRY_LIMIT ) {
                                ClientConnectRequest connect_request;
                                NetworkClient::sendMessage( &connect_request, sizeof(ClientConnectRequest));
                                time_out_counter++;
                            } else {
                                LoadingView::append( "Connection To Server Failed" );
                                connection_state = _connect_state_connect_failure;
                                failure_display_timer.reset();
                            }
                        }

                    }

                } // ** if ( message != 0 )
                else {
                    if ( time_out_timer.count() ) {
                        if ( time_out_counter < _CLIENT_CONNECT_RETRY_LIMIT ) {
                            ClientConnectRequest connect_request;
                            NetworkClient::sendMessage( &connect_request, sizeof(ClientConnectRequest));
                            time_out_counter++;
                        } else {
                            LoadingView::append( "Connection To Server Failed" );
                            connection_state = _connect_state_connect_failure;
                        }
                    }

                }

                end_cycle = true;
            }
            break;

        case _connect_state_wait_for_server_game_setup : {
                if ( message != 0 ) {
                    if ( message->message_id == _net_message_id_connect_server_game_setup ) {
                        ConnectMesgClientGameSetupPing client_game_setup_ping;

                        int result_code;
                        char str_buf[128];
                        ConnectMesgServerGameSettings *game_setup;
                        game_setup = (ConnectMesgServerGameSettings *) message;

                        GameManager::startClientGameSetup( message, &result_code );

                        if( result_code == _mapload_result_no_map_file ) {
                            sprintf( str_buf, "MAP %s NOT FOUND!", game_setup->map_name );
                            LoadingView::append( str_buf);
                            connection_state = _connect_state_connect_failure;
                            failure_display_timer.reset();
                        } else
                            if( result_code == _mapload_result_no_wad_file ) {
                                LoadingView::append( "MAP TILE SET NOT FOUND!" );
                                LoadingView::append( "please download the appropriate tileset" );
                                LoadingView::append( "from www.pyrosoftgames.com" );
                                connection_state = _connect_state_connect_failure;
                                failure_display_timer.reset();
                            } else {
                                LoadingView::append( "Loading Game Data ..." );

                                NetworkClient::sendMessage( &client_game_setup_ping, sizeof(ConnectMesgClientGameSetupPing));

                                connection_state = _connect_state_setup_client_game;
                            }
                    }
                }

                end_cycle = true;
            }
            break;

        case _connect_state_setup_client_game : {
                char str_buf[128];
                int percent_complete;

                if ( GameManager::clientGameSetup( &percent_complete ) == false ) {
                    ConnectMesgClientGameSetupAck client_game_setup_ack;

                    sprintf( str_buf, "Loading Game Data ... (%d%%)", percent_complete);
                    LoadingView::update( str_buf );

                    NetworkClient::sendMessage( &client_game_setup_ack, sizeof(ConnectMesgClientGameSetupAck));
                    connection_state = _connect_state_idle;
                } else {
                    ConnectMesgClientGameSetupPing client_game_setup_ping;

                    sprintf( str_buf, "Loading Game Data ... (%d%%)", percent_complete);
                    LoadingView::update( str_buf );
                    NetworkClient::sendMessage( &client_game_setup_ping, sizeof(ConnectMesgClientGameSetupPing));
                }

                end_cycle = true;
            }
            break;


        case _connect_state_connect_failure : {
                if ( failure_display_timer.count() == true ) {
                    LoadingView::loadError();
                    connection_state = _connect_state_idle;
                }
                end_cycle = true;
            }
            break;

        } // ** switch

    } while( end_cycle == false );


}

void ClientConnectDaemon::connectProcess(const NetMessage* message)
{
    connectFsm( message );
}

void ClientConnectDaemon::connectProcess()
{
    connectFsm( 0 );
}
