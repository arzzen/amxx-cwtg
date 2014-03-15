/*******************************************
*  [CW/TG] 3.5.0 plugin for CS 1.6 server  *
*       Created by (c) Lukas Mestan        *
*               2006-2011                  *
********************************************/

/**
* TO DO:
* 1, nastavenie pristupu spravit cez configuracny subor pluginu
*/

#include <amxmodx>
#include <amxmisc>
#include <cstrike>
#include <fun>

// plugin pristupny len pre administratorov
//#define ADMINS_ONLY

// konstanta jazyka
#define CWTG_JAZYK "cwtg_lang.txt"

// konstanta configuracneho suboru
#define CWTG_CONFIG "cwtg_config.ini"

// konstanta map
#define CWTG_MAPY "cwtg_mapy.ini"

// konstanta configov nastavenie servra
#define CWTG_MR15_5on5 "MR15_3on35on5.cfg"
#define CWTG_MR15_1on1 "MR15_1on12on2.cfg"
#define CWTG_MR5_1on1 "MR5_1on12on2.cfg"
#define CWTG_MR5_5on5 "MR5_3on35on5.cfg"
#define CWTG_MOD_ROZOHRA "MOD_rozohra.cfg"
#define CWTG_MOD_KNIFE "MOD_knife.cfg"

// ostatne konstanty
#define PLUGIN "[CW/TG] Plugin"
#define VERSION "3.5.0"
#define AUTHOR "Lukas Mestan"

// konstanta nazvu zvukov
#define ZVUK_GO "go.wav"

// konstanta na odladovanie pluginu
//#define DEBUG 0

enum _teams {
    _terro,
    _ct
}
new G_MAX_MAPS = 128
new G_ADMIN_NAME[] = "ADMIN"
new g_iScore[_teams]
new match_inprogress = 0
new g_menuPosition[33]
new g_menuPlayers[33][32] 
new g_menuPlayersNum[33]
new g_coloredMenus
new g_mapnames[256][32] // change from 128 to G_MAX_MAPS
new g_mapcount
new g_filename[256]
new client_demoname[128]
new rec_pl = 0

/**
 * Initialize plugin
 * 
 * @access public
 * @return void
 */
public void:plugin_init()
{
	register_plugin(PLUGIN, VERSION, AUTHOR)

    // inicializacia suboru s prekladom
	register_dictionary(CWTG_JAZYK)
    
    // nastavenie menu
	register_clcmd("say /menu", "cmd_menu_hl", 0, "- hlavne menu")
	register_clcmd("say_team /menu", "cmd_menu_hl", 0, "- hlavne menu")
	register_menucmd(register_menuid("[CW/TG] HLAVNE MENU"), 1023, "prikaz_cmd_menu_hl")
	//register_clcmd("cfgmenu", "cmd_menu_cfg", 0, "- zobarzi config menu")
	register_menucmd(register_menuid("[CW/TG] CONFIG MENU"), 1023, "prikaz_cmd_menu_cfg")
    
    // vypsis a kick hraca so serveru (menu)
	//register_clcmd("kickmenu", "cmdKickMenu", 0, "- zobarzi kick menu")
	register_menucmd(register_menuid("[CW/TG] KICK MENU"), 1023, "actionKickMenu")
    
    // nahratie screenu u klienta + aj cez menu
	//register_clcmd("say /scr", "prikaz_ss_menu", 0)
	//register_clcmd("say_team /scr", "prikaz_ss_menu", 0)
	//register_clcmd("say /rec", "rec_demo", 0)
	//register_clcmd("say_team /rec", "rec_demo", 0)
	register_menucmd(register_menuid("[CW/TG] SCREEN MENU"), 1023, "prikaz_ss_menu_ank")
	register_menucmd(register_menuid("[CW/TG] NAHRAVANIE DEMA"), 1023, "rec_demo_action")
	register_menucmd(register_menuid("[CW/TG] NENAHRAVANIE DEMA"), 1023, "rec_demo_off_action")
	g_coloredMenus = colored_menus()
    
    // ostatne nastavenia
	//register_clcmd("say /start", "cmd_start", 0, "- 3x restart a start zapasu")
	//register_clcmd("say_team /start", "cmd_start", 0, "- 3x restart a start zapasu")
	//register_clcmd("say /stop", "cmd_stop", 0, "- zastavy zapas")
	//register_clcmd("say_team /stop", "cmd_stop", 0, "- zastavy zapas")
	//register_clcmd("say /rr", "cmd_rr", 0, "- restart kola")
	//register_clcmd("say_team /rr", "cmd_rr", 0, "- restart kola")
	register_clcmd("say /pauza", "pause_server", 0, "- pausa")
	register_clcmd("say_team /pauza", "pause_server", 0, "- pausa")
	register_clcmd("newpw", "change_pass")
	register_clcmd("say /heslo", "show_pass", 0, "- ukaze vsetkym heslo serveru")
	register_clcmd("say_team /heslo", "show_pass", 0, "- ukaze vsetkym heslo serveru")
	register_clcmd("say heslo", "show_pass", 0, "- ukaze vsetkym heslo serveru")
	register_clcmd("say_team heslo", "show_pass", 0, "- ukaze vsetkym heslo serveru")
	register_cvar("allow_pause", "1")
    
    // MAPY
	register_clcmd("say", "check_map", 0, "- zisti ci uzivatel zadal do say mapu ak ano vykona sa ak je v subore cwtg_mapy.ini")
	read_maps()
    
    // COPPYRIGHT + HELP
	//register_clcmd("copyright", "o_plugine")
	//register_clcmd("say /pomoc", "show_help", 0, "- rychle informacie")
	//register_clcmd("say_team /pomoc", "show_help", 0, "- rychle informacie")
	//register_clcmd("say /napoveda", "cmd_showhelp", 0, "- ukaze help v MOTD okne")
	register_clcmd("say /status", "cmd_showstatus", 0, "- ukaze game id status hracov v konzole");
	register_clcmd("say_team /status", "cmd_showstatus", 0, "- ukaze game id status hracov v motd okne");
	//register_clcmd("say_team /napoveda", "cmd_showhelp", 0, "- ukaze help v MOTD okne")
	//register_clcmd("say /napoveda", "cmd_showhelp", 0, "- ukaze help v MOTD okne");
	register_clcmd("prikazy", "more_help")
    
    // UKAZE SCHORE 
	register_event("TeamScore", "Event_TeamScore", "a")
	register_logevent("ClientCommand_SayScore", 2, "1=Round_End")  
    	register_clcmd("say /score", "ClientCommand_SayScore", 0, "- ukaze score v say")
}

/**
 * Prehra zvuk odpocitavania [3,2,1]
 *
 * @access public
 * @param integer id
 * @param array zlozka
 * @param array zvuk
 * @return void
 */
public void:play_sound(const id, const zlozka[], const zvuk[])
{
	client_cmd(id, "spk ^"/%s/%s^"", zlozka, zvuk);
	#if defined DEBUG
		log_message("[CW/TG] DEBUG: hra zvuk: <%s> uzivatelovi id <#%d>", zvuk, id)
	#endif
}

/**
 * Play sound [one]
 * 
 * @access public
 * @return void
 */
public void:play_sound1() 
{
    client_cmd(0, "spk ^"/fvox/one.wav^"")
}

/**
 * Play sound [two]
 * 
 * @access public
 * @return void
 */
public void:play_sound2() 
{
    client_cmd(0, "spk ^"/fvox/two.wav^"")
}

/**
 * Play sound [three]
 * 
 * @access public
 * @return void
 */
public void:play_sound3() 
{
    client_cmd(0, "spk ^"/fvox/three.wav^"")
}

/**
 * Nacita aktualne team skore
 *
 * @access public
 * @return void
 */
public void:Event_TeamScore()
{
    new szTeamName[2]
    read_data(1, szTeamName, 1)
    g_iScore[szTeamName[0] == 'T' ? _terro : _ct] = read_data(2)
}

/**
 * Zobrazi klientovi aktualne team skore
 *
 * @access public
 * @return integer
 */
public void:ClientCommand_SayScore(id)
{
    client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_SCORE", g_iScore[_terro], g_iScore[_ct])
    return PLUGIN_HANDLED
}

// ======================= CONNECT and DISCONNECT
#if defined ADMINS_ONLY
    public void:client_putinserver(id)
    {
	    if(access(id, ADMIN_MENU))
        {
	        if(is_user_bot(id))
	        {
                return			
            }
            
            if(is_user_connected(id))
            {
                if(valid_access(id))
                { 
                    set_task(2.0, "show_help", id)
                }
	        }
	    }
	    else
	    {
            return
        }
    }
    
    public void:client_disconnect(id)
    {
	    remove_task(id)
    }
#else
    public void:client_putinserver(id)
    {
	    if(is_user_bot(id))
	    {
            return
        }
        
	    if(is_user_connected(id))
        {
            if(valid_access(id))
            {
                set_task(2.0, "show_help", id)
            }
	    }
    }
    
    public void:client_disconnect(id)
    {
	    remove_task(id)
    }
#endif

/**
 * ZISTIME CI MA UZIVATEL POZADOVANY retazec V MENE
 * 
 * @access public
 * @param integer id
 * @return boolean
 */
public valid_access(id)
{
    new name[32]
    get_user_name(id, name, 31)
   
    #if defined ADMIN_NAME_SET
        if(contain(name, G_ADMIN_NAME) == -1)
        {
            client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_NOACS")
            console_print(id, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_NOACS")
            remove_task(id)
            return false
        }
        else
        {
            return true
        }
    #else
        return true   
    #endif
}

/**
 * HLAVNE MENU
 * 
 * @access public
 * @param integer id
 * @return integer
 */
public cmd_menu_hl(id)
{
    if( ! valid_access(id)) 
    { 
        return PLUGIN_HANDLED
    }

    new menu[450], pomm[10], pomm1[11]
    new klavesa = MENU_KEY_0|MENU_KEY_1|MENU_KEY_2|MENU_KEY_3|MENU_KEY_4|MENU_KEY_5|MENU_KEY_6|MENU_KEY_7|MENU_KEY_8|MENU_KEY_9
        
    if(match_inprogress != 1)
    {
        pomm = "Start"
    }
    else
    {
        pomm = "Stop"
    }

    pomm1 = "Pauznut"
    format(menu, 450, "\y[CW/TG] %L", LANG_PLAYER, "CWTG_MENU", pomm)
    show_menu(id, klavesa, menu)
      
    return PLUGIN_CONTINUE
}

/**
 * Prikaz pre hlavne ovladacie menu
 *
 * @access public
 * @param integer id
 * @param integer klavesa
 * @param integer menu
 * @return integer
 */
public prikaz_cmd_menu_hl(id, klavesa, menu)
{
    if( ! valid_access(id))
    {
	    return PLUGIN_HANDLED
    }

    new pl_name[32]
    get_user_name(id, pl_name, 31)
    
    switch(klavesa)
    {
        case 0:
            if(match_inprogress != 1)
            {    
                cmd_start(id)
            }
            else
            {    
                cmd_stop(id)
            }
            break        
        case 1:
            client_print(0, print_chat, "[CW/TG] %L.", LANG_PLAYER, "CWTG_RESTART", pl_name)
		    cmd_rr(id)
            break
        case 2:
            client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_PAUSE", pl_name)
		    pause_server(id)
            break
        case 3:
            prikaz_ss_menu(id)
            break
        case 4:
            rec_demo(id)
            break
        case 5:
            displayKickMenu(id, g_menuPosition[id] = 0)
            break
        case 6:
            cmd_menu_cfg(id)
            break
        case 7:
            cmd_showstatus(id)
            break
        case 8:
            o_plugine(id)
            break
    }

    return PLUGIN_HANDLED
}

/**
 * Podmenu pre vyber a nastavenie konfigu zapasu
 * 
 * @access public
 * @param integer id
 * @return boolean|
 */
public cmd_menu_cfg(id)
{
    if( ! valid_access(id))
    {
        return PLUGIN_HANDLED
    }
    
    new menu[252], pomm[31], pomm1[31], pomm2[31], pomm3[31], pomm4[31], pomm5[31]
    new klavesa = MENU_KEY_0|MENU_KEY_1|MENU_KEY_2|MENU_KEY_3|MENU_KEY_4|MENU_KEY_5|MENU_KEY_6|MENU_KEY_7
    
    if(match_inprogress != 1)
    {
        format(pomm, 30, "%L", LANG_PLAYER, "CWTG_MENU1_1")
        format(pomm1, 30, "%L", LANG_PLAYER, "CWTG_MENU1_3")
        format(pomm3, 30, "%L", LANG_PLAYER, "CWTG_MENU1_5")
        format(pomm4, 30, "%L", LANG_PLAYER, "CWTG_MENU1_7")
        format(pomm5, 30, "%L", LANG_PLAYER, "CWTG_MENU1_9")
        format(pomm2, 30, "%L", LANG_PLAYER, "CWTG_MENU1_11")
    }
    else
    {
        format(pomm, 30, "%L", LANG_PLAYER, "CWTG_MENU1_2")
        format(pomm1, 30, "%L", LANG_PLAYER, "CWTG_MENU1_4")
        format(pomm3, 30, "%L", LANG_PLAYER, "CWTG_MENU1_6")
        format(pomm4, 30, "%L", LANG_PLAYER, "CWTG_MENU1_8")
        format(pomm5, 30, "%L", LANG_PLAYER, "CWTG_MENU1_10")
        format(pomm2, 30, "%L", LANG_PLAYER, "CWTG_MENU1_12")
    }	
    
    format(menu, 251, "\y[CW/TG] %L", LANG_PLAYER, "CWTG_MENU1", pomm, pomm1, pomm3, pomm4, pomm5, pomm2)
    show_menu(id, klavesa, menu)
    
    return PLUGIN_CONTINUE
}

/**
 * Prikaz na zmenu konfigu zapasu
 *
 * @access public
 * @param integer id
 * @param integer klavesa
 * @param integer menu
 * @return integer
 */
public prikaz_cmd_menu_cfg(id, klavesa, menu)
{
    if( ! valid_access(id))
    {
	 return PLUGIN_HANDLED
    }

    new pl_name[32]
    get_user_name(id, pl_name, 31)

    switch( klavesa )
    {
        case 0:
            client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CFG_1", pl_name)
	        cfg_set(id,1)
	        cmd_start(id)
	        return PLUGIN_HANDLED
 	        break
	    case 1:
	        client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CFG_2", pl_name)
	        cfg_set(id,2)
	        cmd_start(id)
	        return PLUGIN_HANDLED
	        break
	    case 2:
	        client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CFG_3", pl_name)
	        cfg_set(id,4)
	        cmd_start(id)
	        return PLUGIN_HANDLED
	        break
	    case 3:
	        client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CFG_4", pl_name)
	        cfg_set(id,5)
	        cmd_start(id)
	        return PLUGIN_HANDLED
	        break
	    case 4:
	        client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CFG_5", pl_name)
	        cfg_set(id,6)
	        cmd_rr(id)
	        return PLUGIN_HANDLED
	        break
	    case 5:
	        client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CFG_6", pl_name)
	        cfg_set(id,3)
	        cmd_rr(id)
	        return PLUGIN_HANDLED
	        break
	    default:
	        return PLUGIN_HANDLED
	        break
    }
}

/**
 * Spusti a vykona nastavenie noveho konfiguracneho nastavenia
 *
 * @access public
 * @param integer id
 * @param integer what
 * @return integer
 */
public int:cfg_set(id, what)
{
   if( ! valid_access(id))
   {
        return PLUGIN_HANDLED
   }

   if(match_inprogress != 1)
   {
	    switch( what )
	    {
	        case 1:
		        server_cmd("exec addons/amxmodx/configs/cwtg/%s", CWTG_MR15_5on5)
		        return PLUGIN_HANDLED
		        break
	        case 2:
	  	        server_cmd("exec addons/amxmodx/configs/cwtg/%s", CWTG_MR15_1on1)
		        return PLUGIN_HANDLED
		        break
	        case 3:
  		        server_cmd("exec addons/amxmodx/configs/cwtg/%s", CWTG_MOD_ROZOHRA)
		        return PLUGIN_HANDLED
		        break
	        case 4:
  		        server_cmd("exec addons/amxmodx/configs/cwtg/%s", CWTG_MR5_5on5)
      		    return PLUGIN_HANDLED
		        break
	        case 5:
	  	        server_cmd("exec addons/amxmodx/configs/cwtg/%s", CWTG_MR5_1on1)
      	        return PLUGIN_HANDLED
		        break
	        case 6:
	  	        server_cmd("exec addons/amxmodx/configs/cwtg/%s", CWTG_MOD_KNIFE)
      		    return PLUGIN_HANDLED
		        break
	        default:
    		    client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_NOREA")
      		    return PLUGIN_HANDLED
		    break
	    }
    }

    return PLUGIN_CONTINUE
}

/**
 * Spusti zapas
 *
 * @access public
 * @param integer id
 * @return integer
 */
public cmd_start(id)
{
   if( ! valid_access(id))
   {
       return PLUGIN_HANDLED
   }

   new pl_name[32], text[255]
   get_user_name(id, pl_name, 31)

   if(match_inprogress != 1)
   {
	    set_task(1.0, "restart_round", 0, "1", 1)
	    set_task(1.1, "play_sound3", 0, "", 1)
	    set_task(3.1, "play_sound2", 0, "", 1)
	    set_task(5.1, "play_sound1", 0, "", 1)
	    
        format(text, 254, "%L", LANG_PLAYER, "CWTG_STARTM1")
	    all_msg(text)
	    match_inprogress = 1
	    
        set_task(3.0, "restart_round", 0, "1", 1)
	    set_task(5.0, "restart_round", 0, "3", 1)
	    set_task(9.0, "live_msg")
	    
        client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_STARTM", pl_name)
   }
   else
   {
	    client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_NOSTP")
   }

   return PLUGIN_CONTINUE
}

/**
 * Zastavy spusteny zapas
 * 
 * @access public
 * @param integer id
 * @return integer
 */
public cmd_stop(id)
{
   if( ! valid_access(id))
   {
       return PLUGIN_HANDLED
   }	

   new pl_name[32], text[255]
   get_user_name(id, pl_name, 31)
	
   if(match_inprogress == 1)
   {
	    format(text, 254, "%L", LANG_PLAYER, "CWTG_STOPM1")
	    all_msg(text)
	    match_inprogress = 0
	    set_task(3.0, "restart_round", 0, "1", 1)
	    format(text, 25, "%L", LANG_PLAYER, "CWTG_STOPM2")
	    set_task(6.0, "all_msg", 0, text, 25)
	    client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_STOPM", pl_name)
   }
   else
   {
	    client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_NOSTP1")
   }

   return PLUGIN_CONTINUE
}

/**
 * Restartuje zapas
 * 
 * @access public
 * @param integer id
 * @return integer
 */
public cmd_rr(id)
{
   if( ! valid_access(id))
   {
       return PLUGIN_HANDLED
   }
	
   if(match_inprogress != 1)
   {
	    set_task(1.0, "restart_round", 0, "1", 1)
	    all_msg("restart")
   }
   else
   {
	    client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_NORR")
   }

   return PLUGIN_CONTINUE
}

/**
 * Nacitanie zoznamu map
 * 
 * @access public
 * @return void
 */
public void:read_maps()
{
   new textlength
   get_filename(CWTG_MAPY)
   new line = 0
	
   while(line < G_MAX_MAPS && read_file(g_filename, line, g_mapnames[line], 30, textlength))
   {
	    ++line
   }

   log_message("[CW/TG] Nacitanych map: %i", line)
   g_mapcount = line
}

/**
 * Overi ci zadany prikaz pre zmenu mapy je platny
 * ak je platny prikaz zmeni sa mapa
 *
 * @access public
 * @param integer id
 * @return void
 */
public void:check_map(id)
{
    new said[192]
    read_args(said, 191)
    new i = 0
	
    while(i < g_mapcount)
    {
	    new trash[16], mapname[32], mapname1[32]
	    strtok(g_mapnames[i], trash, 15, mapname, 31, '_')
	    format(mapname, 31, "^"/%s^"", mapname)
	    format(mapname1, 31, "^"/%s^"", g_mapnames[i])

	    if(equali(said,mapname) || equali(said,mapname1))
	    {
	        cmd_changemap(id, g_mapnames[i])
	    }
	    ++i
    }
}

/**
 * Zmeni mapu na servery 
 *
 * @access public
 * @param integer id
 * @param array mapname
 * @return integer
 */
public int:cmd_changemap(id, mapname[])
{	
    if( ! valid_access(id))
    {
        return PLUGIN_HANDLED
    }	

    if(match_inprogress != 1)
    {
	    new pl_name[32]
	    new message[64]
	    get_user_name(id, pl_name, 31)
	    set_task(2.0, "change_map", 0, mapname, strlen(mapname))
	    format(message, 63, "[CW/TG] %L", LANG_PLAYER, "CWTG_CHMAP1", mapname)
	    all_msg(message)
	
	    //if(mapname!="" || mapname!="\"){
	    client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_CHMAP", pl_name, mapname)
	    /*
    	}else{
      	    client_print(0, print_chat, "[CW/TG] !!! Error: Nemoze sa zmenit mapa.");
    	}
    	*/
    }
    else
    {
	    client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_ERR_CHMAP")
    }

    return PLUGIN_CONTINUE
}

/**
 * Inicializuje zoznam dostupnych map na servery
 *
 * @access public
 * @param array filename
 * @return integer
 */
public get_filename(filename[])
{
    new dir[128]
    get_configsdir(dir, 127)
    format(g_filename, 255, "%s/cwtg/%s", dir, filename)
	
    if( ! file_exists(g_filename))
    {
	    log_message("[CW/TG] Subor %s sa nenasiel.", filename)
	    return PLUGIN_HANDLED
    }

    return PLUGIN_CONTINUE
}

/**
 * Zmeni mapu na servery 
 * 
 * @access public
 * @param array map
 * @return integer
 */
public change_map(map[])
{
    server_cmd("changelevel %s", map)
    return PLUGIN_CONTINUE
}

/**
 * Restartuje server 
 * 
 * @access public
 * @param array seconds
 * @return integer
 */
public restart_round(seconds[])
{
    server_cmd("sv_restartround %s", seconds)
    return PLUGIN_CONTINUE
}

/**
 * Pauzne server
 *
 * @access public
 * @param integer id
 * @return integer
 */
public pause_server(id)
{
    if( ! valid_access(id))
    {
        return PLUGIN_HANDLED
    }
	
    if( ! get_cvar_num("allow_pause"))
    {
	    return PLUGIN_CONTINUE
    }

    client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_PAUSESRV")
    set_cvar_num("pausable", 1)
    client_cmd(id, "pause")
   
    return PLUGIN_CONTINUE
}

/**
 * Vypise spracu do stredu obrazovky
 *
 * @access public
 * @return integer
 */
public live_msg()
{
	set_hudmessage(255, 0, 0, -1.0, 0.2, 0, 6.0, 6.0)
	show_hudmessage(0, "%L", LANG_PLAYER, "CWTG_LIVE1")
	client_print(0, print_chat, "%L", LANG_PLAYER, "CWTG_LIVE")
	// format(hh, 63, "%L", id, "CWTG_HLASKA_ZPSS")
	// client_print(0, print_chat, "%L", "CWTG_HLASKA_ZPSS")
	client_print(0, print_chat, "%L", LANG_PLAYER, "CWTG_LIVE")
	// client_print(0, print_chat, "%L", "CWTG_HLASKA_ZPSS")
	return PLUGIN_CONTINUE
}

/**
 * Vypise spravu/y zadane v parametri
 * do stredu obrazovky 
 *
 * @access public
 * @param array msg
 * @return integer
 */
public all_msg(msg[])
{
	set_hudmessage(255, 0, 0, -1.0, 0.2, 0, 6.0, 6.0)
	show_hudmessage(0, "--- %s ---", msg)
	client_print(0, print_chat, "%L", LANG_PLAYER, "CWTG_MSG", msg)
	return PLUGIN_CONTINUE
}

/**
 * Vypise text o programe
 *
 * @access public
 * @param integer id
 * @return integer
 */
public show_help(id)
{
	client_print(0, print_chat, "[CW/TG] Vytvoril Lukas Mestan, (c) 2007-2009.")
	client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_HELP1")
	client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_HELP2")
	return PLUGIN_CONTINUE
}

/**
 * Vypise napovedu do konzoly
 *
 * @access public
 * @param integer id
 * @return integer
 */
public more_help(id)
{
	console_print(id, "===================================================")
	console_print(id, "                              [CW/TG] Plugin %s", VERSION)
	console_print(id, "                         Vytvoril Lukas Mestan, (c) 2007-2009")
	#if defined ADMIN_NAME_SET
        console_print(id, "===================================================")
        console_print(id, "                              Popis ovladania serveru")
        console_print(id, "===================================================")
        console_print(id, "- pre spravnu funkcnost je potrebne mat za menom v hre slovo ADMIN")
        console_print(id, "- ovladanie pluginu je intuitivne pomocou say (say_team) a konzolovych prikazov" )   
    #endif
    console_print(id, "===================================================")
	console_print(id, "                                 Say/Say_team prikazy")
	console_print(id, "===================================================")
	console_print(id, " say /menu - hlavne menu pre ovladanie serveru")
	console_print(id, " say /start - spusti zapas")
	console_print(id, " say /stop - zastavy zapas")
	console_print(id, " say /rr - 1 krat restart kola")
	console_print(id, " say /pauza - prvy krat pauzne hru, druhy krat unpauzne")
	console_print(id, " say /scr - u klienta odfoti score tabulku")
	console_print(id, " say /rec - u klienta spusti nahravanie in-eye dema")
	console_print(id, " say /pomoc - rychle informacie o ovladani serveru")
	/*
	console_print(id, " say /default - na servery nastavy config na 3na3/5na5 zapas")
	console_print(id, " say /knife - na servery nastavy config na 1na1/2na2 zapas")
	console_print(id, " say /rozohra - nastavy server pre rozohru")
	*/
	console_print(id, " say /score - ukaze aktualne score")
	console_print(id, " say /heslo - zobrazi aktualne heslo servera")
	console_print(id, " say /status - vypise game id status hracov v konzole")
	console_print(id, "===================================================")
	console_print(id, "                                 Konzolove prikazy")
	console_print(id, "===================================================")
	console_print(id, " prikazy - zoznam vsetkych dostupnych prikazov")
	console_print(id, " newpw - nastavi nove heslo na server")
	console_print(id, " kickmenu - menu pre kick hraca so serveru")
	console_print(id, " cfgmenu - menu pre zmenu configu")
	console_print(id, " copyright - informacia o plugine")
	console_print(id, "===================================================")
	console_print(id, "                              Prikazy na zmenu mapy")
	console_print(id, "===================================================")
	
    new i = 0
	while(i < g_mapcount)
    {
		new mapcmd[128], trash[16], mapname[32]
		
        strtok(g_mapnames[i], trash, 15, mapname, 31, '_')
		format(mapcmd, 127, " say /%s alebo /%s", mapname, g_mapnames[i])
        console_print(id, mapcmd)
		
        ++i
	}

	console_print(id, "===================================================")
	console_print(id, "(za akekolvek poskodenie serveru sposobene pluginom autor nezodpoveda)")
	
    return PLUGIN_CONTINUE
}

/**
 * Vypise zakladnu napovedu do MOTD okna
 *
 * @access public
 * @param integer id
 * @return integer
 */
public cmd_showhelp(id)
{
	new motd[8048], line[513], title[64]
	add(motd, 8047, "<html><head><style>")
	add(motd, 8047, "body {background-color: #000000; color: #FFFFFF; font-family: Tahoma; font-size: 12px}")
	add(motd, 8047, "code {font-size: 14px; margin: 0 10px 0 0; color: #FFFFFF;}")
	add(motd, 8047, "h1 {font-size: 18px; text-align: center; color: #FFFFFF;}")
	add(motd, 8047, "h2 {font-size: 14px; font-weight: bold; color: #FFFFFF;}")
	add(motd, 8047, "p {text-align: center; margin: 5px; color: #FFFFFF;}")
	add(motd, 8047, "a {color: orange}")
	add(motd, 8047, "</style></head><body>")
	add(motd, 8047, "<div>")
	format(line, 255, "<h1>[CW/TG] Plugin %s</h1>", VERSION)
	add(motd, 8047, line)
	format(line, 512, "<p>Autor: <strong>%s</strong></p>", AUTHOR)
	add(motd, 8047, line)
	format(line, 512, "<p>Web: <a href=http://www.gameszone.sk>www.gameszone.sk</a></p>")
	add(motd, 8047, line)
	format(line, 512, "<h2>Zoznam say/say_team prikazov:</h2>")
	add(motd, 8047, line)
	format(line, 512, "<ul><li><code>say /menu</code> - hlavne menu pre ovladanie serveru</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /start</code> - spusti cwtg zapas</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /stop</code> - zastavy cwtg zapas</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /rr</code> - 1 krat restartne kolo</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /pauza</code> - prvy krat pauzne a druhy krat unpauzne server</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /status</code> - vypise game id status hracov v konzole</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /heslo</code> - zobrazi aktualne heslo serveru</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /scr</code> - u klienta odfoti screen score</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /rec</code> - u klienta spusti nahravanie in-eye dema</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /score</code> - zobrazi aktualne score</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>say /napoveda</code> - zobrazi uplny zoznam dostupnych prikazov v MOTD</li></ul>")
	add(motd, 8047, line)
	format(line, 512, "<h2>Zoznam konzolovych prikazov:</h2>")
	add(motd, 8047, line)
	format(line, 512, "<ul><li><code>prikazy</code> - zoznam prikazov</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>kickmenu</code> - kick menu</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>cfgmenu</code> - config menu</li>")
	add(motd, 8047, line)
	format(line, 512, "<li><code>newpw</code> - nastavi nove heslo serveru</li>")
	add(motd, 8047, line)
	format(line, 512, "</ul>")
	add(motd, 8047, line)
	add(motd, 8047, "</div></body></html>")
	format(title, 63, "%s %s", PLUGIN, VERSION)
	show_motd(id, motd, title)

	return PLUGIN_HANDLED
}

/**
 * AKcia pre kickuntie hraca so servera
 *
 * @access public
 * @param integer id
 * @param integer key
 * @return integer
 */
public actionKickMenu(id, key)
{
    if( ! valid_access(id))
	{
         return PLUGIN_HANDLED
	}

    switch(key)
    {
		case 8: displayKickMenu(id, ++g_menuPosition[id])
		case 9: displayKickMenu(id, --g_menuPosition[id])
		default:
		{
			new player = g_menuPlayers[id][g_menuPosition[id] * 8 + key]
			new authid[32], authid2[32], name[32], name2[32]
			get_user_authid(id, authid, 31)
			get_user_authid(player, authid2, 31)
			get_user_name(id, name, 31)
			get_user_name(player, name2, 31)
			new userid2 = get_user_userid(player)
			log_amx("[CW/TG] Kick: ^"%s<%d><%s><>^" kick ^"%s<%d><%s><>^"", name, get_user_userid(id), authid, name2, userid2, authid2)
			client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_KICK", name, name2)
			server_cmd("kick #%d", userid2)
			server_exec()
			displayKickMenu(id, g_menuPosition[id])
		}
	}

	return PLUGIN_HANDLED
}

/**
 * Zobrazi menu pre kicnutie hracov so server
 *
 * @access public
 * @param ingteger id
 * @param integer pos
 * @return integer
 */
displayKickMenu(id, pos)
{
    if(pos < 0) 
    {
        return
	}

    get_players(g_menuPlayers[id], g_menuPlayersNum[id])
	new menuBody[512]
    //new menu_txt[512], menu1_txt[512]
	new b = 0
	new i
	new name[32]
	new start = pos * 8 
	
    if(start >= g_menuPlayersNum[id]) 
    {
        start = pos = g_menuPosition[id] = 0
	}
    
    //format(menu_txt, 511, "\y[CW/TG] %L", LANG_PLAYER, "CWTG_MENU2")
	//format(menu1_txt, 511, "[CW/TG] %L", LANG_PLAYER, "CWTG_MENU2_3")
	new len = format(menuBody, 511, g_coloredMenus ? "\y[CW/TG] KICK MENU \R%d/%d^n\w^n" : "[CW/TG] KICK MENU %d/%d^n^n", id, pos + 1, (g_menuPlayersNum[id] / 8 + ((g_menuPlayersNum[id] % 8) ? 1 : 0)))
	new end = start + 8
	new keys = MENU_KEY_0
	
    if(end > g_menuPlayersNum[id])
    { 
        end = g_menuPlayersNum[id]
	}

    for(new a = start; a < end; ++a)
    {
		i = g_menuPlayers[id][a]
		get_user_name(i, name, 31)

		if(access(i, ADMIN_IMMUNITY))
        {
			++b
			if(g_coloredMenus)
			{
            	len += format(menuBody[len], 511-len, "\d%d. %s^n\w", b, name)
			}
            else
			{
            	len += format(menuBody[len], 511-len, "#. %s^n", name)
		    }
        }
        else
        {
			keys |= (1<<b)
			
            if (is_user_admin(i))
			{
        		len += format(menuBody[len], 511-len, g_coloredMenus ? "\r%d.\w %s \r*^n\w" : "%d. %s *^n", ++b, name)
			}
        	else
			{
        		len += format(menuBody[len], 511-len, "\r%d.\w %s^n", ++b, name)
		    }
        }
	}

	if (end != g_menuPlayersNum[id])
    {
		format(menuBody[len], 511-len, "%L", LANG_PLAYER, "CWTG_MENU2_1", id, id)
		keys |= MENU_KEY_9
	}
    else
    {
    	format(menuBody[len], 511-len, "%L", LANG_PLAYER, "CWTG_MENU2_2", id)
	}

    show_menu(id, keys, menuBody, -1, "[CW/TG] KICK MENU")
}

/**
 * Prikaz na kicnutie hraca
 *
 * @access public
 * @param integer id
 * @param integer level
 * @param integer cid
 * @return integer
 */
public cmdKickMenu(id, level, cid)
{
	if( ! valid_access(id))
	{
        return PLUGIN_HANDLED
	}
    
    //if (cmd_access(id, level, cid, 1))
	displayKickMenu(id, g_menuPosition[id] = 0)
	
    return PLUGIN_HANDLED
}

/**
 * Vykona u uzivatela zobrazenie screen score
 *
 * @access public
 * @param array id
 * @return void
 */
public void:rec_ss2(id[])
{
	client_cmd(id[0], "snapshot")
}

/**
 * Vykona u uzivatela skrytie zobrazenia score
 *
 * @access public
 * @param array id
 * @return void
 */
public void:rec_ss3(id[]) 
{
	client_cmd(id[0], "-showscores")
}

/**
 * Udalost na spustenie spravenia screenu uzivatela
 *
 * @access public
 * @param integer id
 * @return integer
 */
public prikaz_ss_menu(id)
{
	if(!valid_access(id))
	{
		return PLUGIN_HANDLED
	}

	new menu[192]
	new klavesa = MENU_KEY_0|MENU_KEY_1|MENU_KEY_2
	format(menu, 191, "\y[CW/TG] %L", LANG_PLAYER, "CWTG_MENU3")
	show_menu(id, klavesa, menu)
	
	return PLUGIN_CONTINUE
}

/**
 * Nahratie screenu u klienta ktory vybral moznost spravenia screenu
 * 
 * @access public
 * @param integer id
 * @param integer klavesa
 * @param integer menu
 * @return integer 
 */
public prikaz_ss_menu_ank(id, klavesa, menu)
{
	if(klavesa == 0)
	{
		new params[1]
		params[0] = id
		new datum[64], cas[64]
		new pl_name[32]
		get_user_name(id, pl_name, 31)
		get_time("%d.%m.%Y" , datum, 63)
		get_time("%H:%M:%S" , cas, 63)
		client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_SCREEN", pl_name, datum, cas)
		client_cmd(id, "+showscores")
		set_task(0.3, "rec_ss2", 0, params, 1)
		set_task(0.6, "rec_ss3", 0, params, 1)
		
		return PLUGIN_HANDLED
	}

	if(klavesa == 1)
	{
		//client_cmd(id, "say_team /menu")
		return PLUGIN_HANDLED
	}
	else // pre istotu toto je pre vsetky tlacitka  > = 3
	{
		return PLUGIN_HANDLED
	}

	return PLUGIN_HANDLED
}

/**
 * Menu pre nahravanie dema 
 * 
 * @access public
 * @param integer id
 * @return integer
 */
public rec_demo(id)
{
	if(!valid_access(id))
	{
		 return PLUGIN_HANDLED
	}

	if(rec_pl == 1)
	{
		new menu[512]
		new klavesa = MENU_KEY_0|MENU_KEY_1|MENU_KEY_2
		format(menu, 511, "\y[CW/TG] %L", LANG_PLAYER, "CWTG_MENU4")
		show_menu(id, klavesa, menu) //ask to stop recording
		rec_pl = 0
		
		return PLUGIN_HANDLED
	}
	else
	{
		new curmap[32]
		new pl_name[32]
		new menu[512]
		new klavesa = MENU_KEY_0|MENU_KEY_1|MENU_KEY_2
		get_user_name(id, pl_name, 31)
		get_mapname(curmap, 31)
		new demoname[256]
		new dem_num = 1
		new stime[64]
		get_time("%d-%m-%Y %H_%M", stime, 63)
		format(demoname, 255, "(%s) %s", pl_name, curmap)
		replace_all(demoname, 64, "/", "-")
		//replace_all(demoname, 64, "\", "-")
		replace_all(demoname, 64, '\\', '-')
		replace_all(demoname, 64, ":", "-")
		replace_all(demoname, 64, "*", "-")
		replace_all(demoname, 64, "?", "-")
		replace_all(demoname, 64, ">", "-")
		replace_all(demoname, 64, "<", "-")
		replace_all(demoname, 64, "|", "-")
		replace_all(demoname, 64, ".", "-")
		
		if( ! is_dedicated_server())
		{
			format(client_demoname, 255, "%s %d.dem", demoname, dem_num)
			while(file_exists(client_demoname))
			{
				dem_num++
				format(client_demoname, 255, "%s %d.dem", demoname, dem_num)
			}
		}
		else
		{	
			format(client_demoname, 255, "%s %s.dem", demoname, stime)
		}

		format(menu, 511, "\y[CW/TG] %L", LANG_PLAYER, "CWTG_MENU4_1", client_demoname)
		show_menu(id, klavesa, menu)
		rec_pl = 1
		// show_menu(id, keys, menu_body, -1, "Record Demo")
	}
	
	return PLUGIN_HANDLED
}

/**
 * Spustenie nahravanie videa 
 *
 * @access public
 * @param integer id
 * @param integer klavesa
 * @param integer menu
 * @return integer
 */
public rec_demo_action(id, klavesa, menu)
{
	if( ! valid_access(id))
	{
		return PLUGIN_HANDLED
	}

	if(klavesa == 0)
	{
		new pl_name[32]
		get_user_name(id, pl_name, 31)
		client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_DEMO1", pl_name, client_demoname)
		client_cmd(id, "stop;record ^"%s^"", client_demoname)
		set_task(3.0, "show_status_cmd", id)
		rec_pl = 1
		// rec_demo_off(id)
		
		return PLUGIN_HANDLED
	}

	if(klavesa == 1)
	{
		// client_cmd(id, "say_team /menu")
		return PLUGIN_HANDLED
	}
	else // pre istotu toto je pre vsetky tlacitka  > = 3
	{
		return PLUGIN_HANDLED
	}

	return PLUGIN_HANDLED
}

/**
 * Vypnutie nahravanie videa
 *
 * @access public
 * @param integer id
 * @param integer klavesa
 * @param menu
 * @return integer
 */
public rec_demo_off_action(id, klavesa, menu)
{
	if( ! valid_access(id))
	{
		return PLUGIN_HANDLED
	}

	if(klavesa == 0)
	{
		new pl_name[32]
		get_user_name(id, pl_name, 31)
		client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_DEMO2", pl_name, client_demoname)
		client_cmd(id, "stop")
		rec_pl = 0
		
		return PLUGIN_HANDLED
	}

	if(klavesa == 1)
	{
		//client_cmd(id, "say_team /menu")
		return PLUGIN_HANDLED
	}
	else // pre istotu toto je pre vsetky tlacitka  > = 3
	{
		return PLUGIN_HANDLED
	}

	return PLUGIN_HANDLED
}

/**
 * Ukaze status v konzole uzivatela
 *
 * @access public
 * @param integer id
 * @return void 
 */
public void:show_status_cmd(id)
{
	client_cmd(id, "status")
}

/**
 * Vypise status 
 *
 * @access public
 * @param integer id
 * @return integer
 */
public cmd_showstatus(id)
{
	if( ! valid_access(id))
	{
		return PLUGIN_HANDLED
  	}

	server_cmd("sv_lan 1")
	server_exec()
	client_cmd(id, "status")
	server_cmd("sv_lan 0")
	server_exec()
	client_print(id, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_STATUS")

	return PLUGIN_HANDLED
}

/**
 * Zmeni heslo pre vstup na server
 * 
 * @access public
 * @param integer id
 * @return integer
 */
public change_pass(id)
{
	if( ! valid_access(id))
	{
		return PLUGIN_HANDLED
	}

	new tmp[32]

	if(read_argc() < 2)
	{
		console_print(id, "[CW/TG] %L", LANG_PLAYER, "CWTG_CON1")
		console_print(id, "[CW/TG] %L", LANG_PLAYER, "CWTG_CON2")
	}
	else
	{
		read_argv(1, tmp, 31)
		//argument = str_to_num(tmp)
		set_cvar_string("sv_password",tmp)
		client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_PW1", tmp)
		console_print(0, "[CW/TG] %L", LANG_PLAYER, "CWTG_CON3", tmp)
		server_exec()
	}

	return PLUGIN_HANDLED
}

/**
 * Vypise zmenene heslo vsetkym pripojenym hracom na servery
 * 
 * @access public
 * @param integer id
 * @return integer
 */
public show_pass(id)
{
	if( ! valid_access(id))
	{
		return PLUGIN_HANDLED
  	}

	new old_pass[128]
  	get_cvar_string("sv_password",old_pass,127)
  	client_print(0, print_chat, "[CW/TG] %L", LANG_PLAYER, "CWTG_PW2", old_pass)
  	
	return PLUGIN_HANDLED
}

