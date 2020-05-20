; ModuleID = 'mosquitto.bc'
source_filename = "mosquitto.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.mosquitto_db = type { i64, %struct.mosquitto__subhier*, %struct.mosquitto__unpwd*, %struct.mosquitto__unpwd*, %struct.mosquitto*, %struct.mosquitto*, %struct.mosquitto*, %struct.mosquitto**, %struct.clientid__index_hash*, %struct.mosquitto_msg_store*, %struct.mosquitto_msg_store_load*, i32, i32, i64, i8*, %struct.mosquitto__config*, i32, i8, i32, i32, i32, i32, %struct.mosquitto*, i32 }
%struct.mosquitto__subhier = type { %struct.UT_hash_handle, %struct.mosquitto__subhier*, %struct.mosquitto__subhier*, %struct.mosquitto__subleaf*, %struct.mosquitto__subshared*, %struct.mosquitto_msg_store*, i8*, i16 }
%struct.UT_hash_handle = type { %struct.UT_hash_table*, i8*, i8*, %struct.UT_hash_handle*, %struct.UT_hash_handle*, i8*, i32, i32 }
%struct.UT_hash_table = type { %struct.UT_hash_bucket*, i32, i32, i32, %struct.UT_hash_handle*, i64, i32, i32, i32, i32, i32 }
%struct.UT_hash_bucket = type { %struct.UT_hash_handle*, i32, i32 }
%struct.mosquitto__subleaf = type { %struct.mosquitto__subleaf*, %struct.mosquitto__subleaf*, %struct.mosquitto*, i32, i8, i8, i8 }
%struct.mosquitto__subshared = type { %struct.UT_hash_handle, i8*, %struct.mosquitto__subleaf* }
%struct.mosquitto__unpwd = type { i8*, i8*, i32, i32, i8*, %struct.UT_hash_handle }
%struct.clientid__index_hash = type opaque
%struct.mosquitto_msg_store = type { %struct.mosquitto_msg_store*, %struct.mosquitto_msg_store*, i64, i8*, i8*, %struct.mosquitto__listener*, i8**, i32, i32, i8*, %struct.mqtt5__property*, %union.mosquitto__payload_uhpa, i64, i32, i16, i16, i8, i8, i8 }
%struct.mosquitto__listener = type { i32, i16, i8*, i8*, i32, i8*, i32*, i32, i32, i32, i32, i8, i8, i16, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct.ssl_ctx_st*, i8*, i8*, i8*, i8, i8, i8, i32, %struct.mosquitto__security_options, %struct.mosquitto__unpwd*, %struct.mosquitto__unpwd* }
%struct.ssl_ctx_st = type opaque
%struct.mosquitto__security_options = type { %struct.mosquitto__acl_user*, %struct.mosquitto__acl*, i8*, i8*, i8*, %struct.mosquitto__auth_plugin_config*, i32, i8, i8, i8*, i32 }
%struct.mosquitto__acl_user = type { %struct.mosquitto__acl_user*, i8*, %struct.mosquitto__acl* }
%struct.mosquitto__acl = type { %struct.mosquitto__acl*, i8*, i32, i32, i32 }
%struct.mosquitto__auth_plugin_config = type { i8*, %struct.mosquitto_opt*, i32, i8, %struct.mosquitto__auth_plugin }
%struct.mosquitto_opt = type { i8*, i8* }
%struct.mosquitto__auth_plugin = type { i8*, i8*, i32 ()*, i32 (i8**, %struct.mosquitto_opt*, i32)*, i32 (i8*, %struct.mosquitto_opt*, i32)*, i32 (i8*, %struct.mosquitto_opt*, i32, i1)*, i32 (i8*, %struct.mosquitto_opt*, i32, i1)*, i32 (i8*, i32, %struct.mosquitto*, %struct.mosquitto_acl_msg*)*, i32 (i8*, %struct.mosquitto*, i8*, i8*)*, i32 (i8*, %struct.mosquitto*, i8*, i8*, i8*, i32)*, i32 (i8*, %struct.mosquitto*, i8*, i1, i8*, i16, i8**, i16*)*, i32 (i8*, %struct.mosquitto*, i8*, i8*, i16, i8**, i16*)*, i32 (i8**, %struct.mosquitto_opt*, i32)*, i32 (i8*, %struct.mosquitto_opt*, i32)*, i32 (i8*, %struct.mosquitto_opt*, i32, i1)*, i32 (i8*, %struct.mosquitto_opt*, i32, i1)*, i32 (i8*, i32, %struct.mosquitto*, %struct.mosquitto_acl_msg*)*, i32 (i8*, %struct.mosquitto*, i8*, i8*)*, i32 (i8*, %struct.mosquitto*, i8*, i8*, i8*, i32)*, i32 (i8**, %struct.mosquitto_auth_opt*, i32)*, i32 (i8*, %struct.mosquitto_auth_opt*, i32)*, i32 (i8*, %struct.mosquitto_auth_opt*, i32, i1)*, i32 (i8*, %struct.mosquitto_auth_opt*, i32, i1)*, i32 (i8*, i8*, i8*, i8*, i32)*, i32 (i8*, i8*, i8*)*, i32 (i8*, i8*, i8*, i8*, i32)*, i32 }
%struct.mosquitto_acl_msg = type { i8*, i8*, i64, i32, i8 }
%struct.mosquitto_auth_opt = type { i8*, i8* }
%struct.mqtt5__property = type opaque
%union.mosquitto__payload_uhpa = type { i8* }
%struct.mosquitto_msg_store_load = type { %struct.UT_hash_handle, i64, %struct.mosquitto_msg_store* }
%struct.mosquitto__config = type { i8, i32, i8, i8, i8*, i8, i8, %struct.mosquitto__listener, %struct.mosquitto__listener*, i32, i32, i32, i32, i8, i8*, i8*, %struct._IO_FILE*, i16, i16, i32, i32, i8, i8*, i8*, i8*, i64, i8*, i8, i8, i8, i8, i32, i8, i8*, %struct.mosquitto__bridge*, i32, %struct.mosquitto__security_options }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.mosquitto__bridge = type { i8*, %struct.bridge_address*, i32, i32, i64, i32, i8, i8, i8, i8, i32, %struct.mosquitto__bridge_topic*, i32, i8, i32, i64, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8, i8, i32, i32, i32, i32, i32, i32, i8, i8, i8, i8, i8, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8* }
%struct.bridge_address = type { i8*, i32 }
%struct.mosquitto__bridge_topic = type { i8*, i32, i32, i8*, i8*, i8*, i8* }
%struct.mosquitto = type { i32, i32, i8*, i8*, i8*, i8*, i16, i16, i32, i64, i64, i64, %struct.mosquitto__packet, %struct.mosquitto__packet*, %struct.mosquitto__packet*, %struct.mosquitto_message_all*, %struct.mosquitto__alias*, %struct.will_delay_list*, i32, i32, i32, i64, %struct.ssl_st*, %struct.ssl_ctx_st*, i8*, i8*, i8*, i8*, i32 (i8*, i32, i32, i8*)*, i8*, i8*, i8*, i8*, i32, i8, i8, i8, i8*, i8*, i32, i8*, i8, i8, i8, i32, i64, i8, i8, i8, %struct.mosquitto__bridge*, %struct.mosquitto_msg_data, %struct.mosquitto_msg_data, %struct.mosquitto__acl_user*, %struct.mosquitto__listener*, %struct.mosquitto__packet*, %struct.mosquitto__subhier**, %struct.mosquitto__subshared_ref**, i8*, i32, i32, i32, i8, i8, i8, %struct.UT_hash_handle, %struct.UT_hash_handle, %struct.mosquitto*, %struct.session_expiry_list*, i32 }
%struct.mosquitto__packet = type { i8*, %struct.mosquitto__packet*, i32, i32, i32, i32, i32, i16, i8, i8 }
%struct.mosquitto_message_all = type { %struct.mosquitto_message_all*, %struct.mosquitto_message_all*, %struct.mqtt5__property*, i64, i32, i8, %struct.mosquitto_message, i32 }
%struct.mosquitto_message = type { i32, i8*, i8*, i32, i32, i8 }
%struct.mosquitto__alias = type { i8*, i16 }
%struct.will_delay_list = type { %struct.mosquitto*, %struct.will_delay_list*, %struct.will_delay_list* }
%struct.ssl_st = type opaque
%struct.mosquitto_msg_data = type { %struct.mosquitto_client_msg*, %struct.mosquitto_client_msg*, i64, i64, i32, i32, i32, i16 }
%struct.mosquitto_client_msg = type { %struct.mosquitto_client_msg*, %struct.mosquitto_client_msg*, %struct.mosquitto_msg_store*, %struct.mqtt5__property*, i64, i16, i8, i8, i32, i32, i8 }
%struct.mosquitto__subshared_ref = type { %struct.mosquitto__subhier*, %struct.mosquitto__subshared* }
%struct.session_expiry_list = type { %struct.mosquitto*, %struct.session_expiry_list*, %struct.session_expiry_list* }
%struct.passwd = type { i8*, i8*, i32, i32, i8*, i8*, i8* }
%struct.timeval = type { i64, i64 }
%struct.timezone = type { i32, i32 }

@flag_reload = local_unnamed_addr global i8 0, align 1, !dbg !0
@flag_db_backup = local_unnamed_addr global i8 0, align 1, !dbg !683
@flag_tree_print = local_unnamed_addr global i8 0, align 1, !dbg !685
@int_db = common global %struct.mosquitto_db zeroinitializer, align 8, !dbg !687
@.str = private unnamed_addr constant [10 x i8] c"SNAP_NAME\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"mosquitto\00", align 1
@.str.2 = private unnamed_addr constant [5 x i8] c"root\00", align 1
@.str.3 = private unnamed_addr constant [26 x i8] c"Error: Invalid user '%s'.\00", align 1
@.str.4 = private unnamed_addr constant [53 x i8] c"Error setting groups whilst dropping privileges: %s.\00", align 1
@.str.5 = private unnamed_addr constant [50 x i8] c"Error setting gid whilst dropping privileges: %s.\00", align 1
@.str.6 = private unnamed_addr constant [50 x i8] c"Error setting uid whilst dropping privileges: %s.\00", align 1
@.str.7 = private unnamed_addr constant [60 x i8] c"Warning: Mosquitto should not be run as root/administrator.\00", align 1
@.str.8 = private unnamed_addr constant [51 x i8] c"Error setting gid whilst restoring privileges: %s.\00", align 1
@.str.9 = private unnamed_addr constant [51 x i8] c"Error setting uid whilst restoring privileges: %s.\00", align 1
@.str.10 = private unnamed_addr constant [18 x i8] c"Error in fork: %s\00", align 1
@.str.11 = private unnamed_addr constant [20 x i8] c"Error in setsid: %s\00", align 1
@.str.12 = private unnamed_addr constant [10 x i8] c"/dev/null\00", align 1
@.str.13 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@stdin = external local_unnamed_addr global %struct._IO_FILE*, align 8
@.str.14 = private unnamed_addr constant [33 x i8] c"freopen(\22/dev/null\22, \22r\22, stdin)\00", align 1
@.str.15 = private unnamed_addr constant [12 x i8] c"mosquitto.c\00", align 1
@__PRETTY_FUNCTION__.mosquitto__daemonise = private unnamed_addr constant [32 x i8] c"void mosquitto__daemonise(void)\00", align 1
@.str.16 = private unnamed_addr constant [2 x i8] c"w\00", align 1
@stdout = external local_unnamed_addr global %struct._IO_FILE*, align 8
@.str.17 = private unnamed_addr constant [34 x i8] c"freopen(\22/dev/null\22, \22w\22, stdout)\00", align 1
@stderr = external local_unnamed_addr global %struct._IO_FILE*, align 8
@.str.18 = private unnamed_addr constant [34 x i8] c"freopen(\22/dev/null\22, \22w\22, stderr)\00", align 1
@.str.19 = private unnamed_addr constant [3 x i8] c"wt\00", align 1
@.str.20 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.21 = private unnamed_addr constant [33 x i8] c"Error: Unable to write pid file.\00", align 1
@.str.22 = private unnamed_addr constant [31 x i8] c"Error: Couldn't open database.\00", align 1
@.str.23 = private unnamed_addr constant [30 x i8] c"mosquitto version %s starting\00", align 1
@.str.24 = private unnamed_addr constant [6 x i8] c"1.6.7\00", align 1
@.str.25 = private unnamed_addr constant [23 x i8] c"Config loaded from %s.\00", align 1
@.str.26 = private unnamed_addr constant [22 x i8] c"Using default config.\00", align 1
@.str.27 = private unnamed_addr constant [41 x i8] c"Warning: Unable to connect to bridge %s.\00", align 1
@run = common local_unnamed_addr global i32 0, align 4, !dbg !817
@.str.28 = private unnamed_addr constant [33 x i8] c"mosquitto version %s terminating\00", align 1

; Function Attrs: norecurse nounwind readnone uwtable
define nonnull %struct.mosquitto_db* @mosquitto__get_db() local_unnamed_addr #0 !dbg !826 {
  ret %struct.mosquitto_db* @int_db, !dbg !830
}

; Function Attrs: nounwind uwtable
define i32 @drop_privileges(%struct.mosquitto__config* nocapture readonly, i1 zeroext) local_unnamed_addr #1 !dbg !831 {
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %0, metadata !835, metadata !DIExpression()), !dbg !854
  call void @llvm.dbg.value(metadata i1 %1, metadata !836, metadata !DIExpression()), !dbg !855
  %3 = tail call i8* @getenv(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0)) #10, !dbg !856
  call void @llvm.dbg.value(metadata i8* %3, metadata !853, metadata !DIExpression()), !dbg !857
  %4 = icmp eq i8* %3, null, !dbg !858
  br i1 %4, label %8, label %5, !dbg !860

; <label>:5:                                      ; preds = %2
  %6 = tail call i32 @strcmp(i8* nonnull %3, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i64 0, i64 0)) #11, !dbg !861
  %7 = icmp eq i32 %6, 0, !dbg !861
  br i1 %7, label %71, label %8, !dbg !862

; <label>:8:                                      ; preds = %5, %2
  %9 = tail call i32 @geteuid() #10, !dbg !863
  %10 = icmp eq i32 %9, 0, !dbg !865
  br i1 %10, label %11, label %71, !dbg !866

; <label>:11:                                     ; preds = %8
  %12 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %0, i64 0, i32 33, !dbg !867
  %13 = load i8*, i8** %12, align 8, !dbg !867, !tbaa !870
  %14 = icmp eq i8* %13, null, !dbg !881
  br i1 %14, label %63, label %15, !dbg !882

; <label>:15:                                     ; preds = %11
  %16 = tail call i32 @strcmp(i8* nonnull %13, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.2, i64 0, i64 0)) #11, !dbg !883
  %17 = icmp eq i32 %16, 0, !dbg !883
  br i1 %17, label %63, label %18, !dbg !884

; <label>:18:                                     ; preds = %15
  %19 = tail call %struct.passwd* @getpwnam(i8* nonnull %13), !dbg !885
  call void @llvm.dbg.value(metadata %struct.passwd* %19, metadata !837, metadata !DIExpression()), !dbg !887
  %20 = icmp eq %struct.passwd* %19, null, !dbg !888
  %21 = load i8*, i8** %12, align 8, !tbaa !870
  br i1 %20, label %22, label %24, !dbg !890

; <label>:22:                                     ; preds = %18
  %23 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str.3, i64 0, i64 0), i8* %21) #10, !dbg !891
  br label %71, !dbg !893

; <label>:24:                                     ; preds = %18
  %25 = getelementptr inbounds %struct.passwd, %struct.passwd* %19, i64 0, i32 3, !dbg !894
  %26 = load i32, i32* %25, align 4, !dbg !894, !tbaa !896
  %27 = tail call i32 @initgroups(i8* %21, i32 %26) #10, !dbg !898
  %28 = icmp eq i32 %27, -1, !dbg !899
  br i1 %28, label %29, label %34, !dbg !900

; <label>:29:                                     ; preds = %24
  %30 = tail call i32* @__errno_location() #12, !dbg !901
  %31 = load i32, i32* %30, align 4, !dbg !901, !tbaa !903
  %32 = tail call i8* @strerror(i32 %31) #10, !dbg !904
  call void @llvm.dbg.value(metadata i8* %32, metadata !851, metadata !DIExpression()), !dbg !905
  %33 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([53 x i8], [53 x i8]* @.str.4, i64 0, i64 0), i8* %32) #10, !dbg !906
  br label %71, !dbg !907

; <label>:34:                                     ; preds = %24
  %35 = load i32, i32* %25, align 4, !tbaa !896
  br i1 %1, label %36, label %38, !dbg !908

; <label>:36:                                     ; preds = %34
  %37 = tail call i32 @setegid(i32 %35) #10, !dbg !909
  call void @llvm.dbg.value(metadata i32 %37, metadata !852, metadata !DIExpression()), !dbg !912
  br label %40, !dbg !913

; <label>:38:                                     ; preds = %34
  %39 = tail call i32 @setgid(i32 %35) #10, !dbg !914
  call void @llvm.dbg.value(metadata i32 %39, metadata !852, metadata !DIExpression()), !dbg !912
  br label %40

; <label>:40:                                     ; preds = %38, %36
  %41 = phi i32 [ %37, %36 ], [ %39, %38 ]
  call void @llvm.dbg.value(metadata i32 %41, metadata !852, metadata !DIExpression()), !dbg !912
  %42 = icmp eq i32 %41, -1, !dbg !916
  br i1 %42, label %43, label %48, !dbg !918

; <label>:43:                                     ; preds = %40
  %44 = tail call i32* @__errno_location() #12, !dbg !919
  %45 = load i32, i32* %44, align 4, !dbg !919, !tbaa !903
  %46 = tail call i8* @strerror(i32 %45) #10, !dbg !921
  call void @llvm.dbg.value(metadata i8* %46, metadata !851, metadata !DIExpression()), !dbg !905
  %47 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([50 x i8], [50 x i8]* @.str.5, i64 0, i64 0), i8* %46) #10, !dbg !922
  br label %71, !dbg !923

; <label>:48:                                     ; preds = %40
  %49 = getelementptr inbounds %struct.passwd, %struct.passwd* %19, i64 0, i32 2
  %50 = load i32, i32* %49, align 8, !tbaa !924
  br i1 %1, label %51, label %53, !dbg !925

; <label>:51:                                     ; preds = %48
  %52 = tail call i32 @seteuid(i32 %50) #10, !dbg !926
  call void @llvm.dbg.value(metadata i32 %52, metadata !852, metadata !DIExpression()), !dbg !912
  br label %55, !dbg !929

; <label>:53:                                     ; preds = %48
  %54 = tail call i32 @setuid(i32 %50) #10, !dbg !930
  call void @llvm.dbg.value(metadata i32 %54, metadata !852, metadata !DIExpression()), !dbg !912
  br label %55

; <label>:55:                                     ; preds = %53, %51
  %56 = phi i32 [ %52, %51 ], [ %54, %53 ]
  call void @llvm.dbg.value(metadata i32 %56, metadata !852, metadata !DIExpression()), !dbg !912
  %57 = icmp eq i32 %56, -1, !dbg !932
  br i1 %57, label %58, label %63, !dbg !934

; <label>:58:                                     ; preds = %55
  %59 = tail call i32* @__errno_location() #12, !dbg !935
  %60 = load i32, i32* %59, align 4, !dbg !935, !tbaa !903
  %61 = tail call i8* @strerror(i32 %60) #10, !dbg !937
  call void @llvm.dbg.value(metadata i8* %61, metadata !851, metadata !DIExpression()), !dbg !905
  %62 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([50 x i8], [50 x i8]* @.str.6, i64 0, i64 0), i8* %61) #10, !dbg !938
  br label %71, !dbg !939

; <label>:63:                                     ; preds = %15, %11, %55
  %64 = tail call i32 @geteuid() #10, !dbg !940
  %65 = icmp eq i32 %64, 0, !dbg !942
  br i1 %65, label %69, label %66, !dbg !943

; <label>:66:                                     ; preds = %63
  %67 = tail call i32 @getegid() #10, !dbg !944
  %68 = icmp eq i32 %67, 0, !dbg !945
  br i1 %68, label %69, label %71, !dbg !946

; <label>:69:                                     ; preds = %66, %63
  %70 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 4, i8* getelementptr inbounds ([60 x i8], [60 x i8]* @.str.7, i64 0, i64 0)) #10, !dbg !947
  br label %71, !dbg !949

; <label>:71:                                     ; preds = %8, %69, %66, %5, %58, %43, %29, %22
  %72 = phi i32 [ 1, %29 ], [ 1, %43 ], [ 1, %58 ], [ 1, %22 ], [ 0, %5 ], [ 0, %66 ], [ 0, %69 ], [ 0, %8 ]
  ret i32 %72, !dbg !950
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind readonly
declare i8* @getenv(i8* nocapture) local_unnamed_addr #3

; Function Attrs: nounwind readonly
declare i32 @strcmp(i8* nocapture, i8* nocapture) local_unnamed_addr #3

; Function Attrs: nounwind
declare i32 @geteuid() local_unnamed_addr #4

; Function Attrs: nounwind
declare %struct.passwd* @getpwnam(i8* nocapture readonly) local_unnamed_addr #4

declare i32 @log__printf(%struct.mosquitto*, i32, i8*, ...) local_unnamed_addr #5

declare i32 @initgroups(i8*, i32) local_unnamed_addr #5

; Function Attrs: nounwind
declare i8* @strerror(i32) local_unnamed_addr #4

; Function Attrs: nounwind readnone
declare i32* @__errno_location() local_unnamed_addr #6

; Function Attrs: nounwind
declare i32 @setegid(i32) local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @setgid(i32) local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @seteuid(i32) local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @setuid(i32) local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @getegid() local_unnamed_addr #4

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define i32 @restore_privileges() local_unnamed_addr #1 !dbg !951 {
  %1 = tail call i32 @getuid() #10, !dbg !955
  %2 = icmp eq i32 %1, 0, !dbg !957
  br i1 %2, label %3, label %19, !dbg !958

; <label>:3:                                      ; preds = %0
  %4 = tail call i32 @setegid(i32 0) #10, !dbg !959
  call void @llvm.dbg.value(metadata i32 %4, metadata !954, metadata !DIExpression()), !dbg !961
  %5 = icmp eq i32 %4, -1, !dbg !962
  br i1 %5, label %6, label %11, !dbg !964

; <label>:6:                                      ; preds = %3
  %7 = tail call i32* @__errno_location() #12, !dbg !965
  %8 = load i32, i32* %7, align 4, !dbg !965, !tbaa !903
  %9 = tail call i8* @strerror(i32 %8) #10, !dbg !967
  call void @llvm.dbg.value(metadata i8* %9, metadata !953, metadata !DIExpression()), !dbg !968
  %10 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([51 x i8], [51 x i8]* @.str.8, i64 0, i64 0), i8* %9) #10, !dbg !969
  br label %19, !dbg !970

; <label>:11:                                     ; preds = %3
  %12 = tail call i32 @seteuid(i32 0) #10, !dbg !971
  call void @llvm.dbg.value(metadata i32 %12, metadata !954, metadata !DIExpression()), !dbg !961
  %13 = icmp eq i32 %12, -1, !dbg !972
  br i1 %13, label %14, label %19, !dbg !974

; <label>:14:                                     ; preds = %11
  %15 = tail call i32* @__errno_location() #12, !dbg !975
  %16 = load i32, i32* %15, align 4, !dbg !975, !tbaa !903
  %17 = tail call i8* @strerror(i32 %16) #10, !dbg !977
  call void @llvm.dbg.value(metadata i8* %17, metadata !953, metadata !DIExpression()), !dbg !968
  %18 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([51 x i8], [51 x i8]* @.str.9, i64 0, i64 0), i8* %17) #10, !dbg !978
  br label %19, !dbg !979

; <label>:19:                                     ; preds = %0, %11, %14, %6
  %20 = phi i32 [ 1, %6 ], [ 1, %14 ], [ 0, %11 ], [ 0, %0 ]
  ret i32 %20, !dbg !980
}

; Function Attrs: nounwind
declare i32 @getuid() local_unnamed_addr #4

; Function Attrs: nounwind uwtable
define void @mosquitto__daemonise() local_unnamed_addr #1 !dbg !981 {
  %1 = tail call i32 @fork() #10, !dbg !990
  call void @llvm.dbg.value(metadata i32 %1, metadata !986, metadata !DIExpression()), !dbg !991
  %2 = icmp slt i32 %1, 0, !dbg !992
  br i1 %2, label %3, label %8, !dbg !994

; <label>:3:                                      ; preds = %0
  %4 = tail call i32* @__errno_location() #12, !dbg !995
  %5 = load i32, i32* %4, align 4, !dbg !995, !tbaa !903
  %6 = tail call i8* @strerror(i32 %5) #10, !dbg !997
  call void @llvm.dbg.value(metadata i8* %6, metadata !985, metadata !DIExpression()), !dbg !998
  %7 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.10, i64 0, i64 0), i8* %6) #10, !dbg !999
  tail call void @exit(i32 1) #13, !dbg !1000
  unreachable, !dbg !1000

; <label>:8:                                      ; preds = %0
  %9 = icmp eq i32 %1, 0, !dbg !1001
  br i1 %9, label %11, label %10, !dbg !1003

; <label>:10:                                     ; preds = %8
  tail call void @exit(i32 0) #13, !dbg !1004
  unreachable, !dbg !1004

; <label>:11:                                     ; preds = %8
  %12 = tail call i32 @setsid() #10, !dbg !1006
  %13 = icmp slt i32 %12, 0, !dbg !1008
  br i1 %13, label %14, label %19, !dbg !1009

; <label>:14:                                     ; preds = %11
  %15 = tail call i32* @__errno_location() #12, !dbg !1010
  %16 = load i32, i32* %15, align 4, !dbg !1010, !tbaa !903
  %17 = tail call i8* @strerror(i32 %16) #10, !dbg !1012
  call void @llvm.dbg.value(metadata i8* %17, metadata !985, metadata !DIExpression()), !dbg !998
  %18 = tail call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.11, i64 0, i64 0), i8* %17) #10, !dbg !1013
  tail call void @exit(i32 1) #13, !dbg !1014
  unreachable, !dbg !1014

; <label>:19:                                     ; preds = %11
  %20 = load %struct._IO_FILE*, %struct._IO_FILE** @stdin, align 8, !dbg !1015, !tbaa !1018
  %21 = tail call %struct._IO_FILE* @freopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.12, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.13, i64 0, i64 0), %struct._IO_FILE* %20) #10, !dbg !1015
  %22 = icmp eq %struct._IO_FILE* %21, null, !dbg !1015
  br i1 %22, label %23, label %24, !dbg !1019

; <label>:23:                                     ; preds = %19
  tail call void @__assert_fail(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.14, i64 0, i64 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.15, i64 0, i64 0), i32 190, i8* getelementptr inbounds ([32 x i8], [32 x i8]* @__PRETTY_FUNCTION__.mosquitto__daemonise, i64 0, i64 0)) #13, !dbg !1015
  unreachable, !dbg !1015

; <label>:24:                                     ; preds = %19
  %25 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8, !dbg !1020, !tbaa !1018
  %26 = tail call %struct._IO_FILE* @freopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.12, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.16, i64 0, i64 0), %struct._IO_FILE* %25) #10, !dbg !1020
  %27 = icmp eq %struct._IO_FILE* %26, null, !dbg !1020
  br i1 %27, label %28, label %29, !dbg !1023

; <label>:28:                                     ; preds = %24
  tail call void @__assert_fail(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.17, i64 0, i64 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.15, i64 0, i64 0), i32 191, i8* getelementptr inbounds ([32 x i8], [32 x i8]* @__PRETTY_FUNCTION__.mosquitto__daemonise, i64 0, i64 0)) #13, !dbg !1020
  unreachable, !dbg !1020

; <label>:29:                                     ; preds = %24
  %30 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !dbg !1024, !tbaa !1018
  %31 = tail call %struct._IO_FILE* @freopen(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.12, i64 0, i64 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.16, i64 0, i64 0), %struct._IO_FILE* %30) #10, !dbg !1024
  %32 = icmp eq %struct._IO_FILE* %31, null, !dbg !1024
  br i1 %32, label %33, label %34, !dbg !1027

; <label>:33:                                     ; preds = %29
  tail call void @__assert_fail(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.18, i64 0, i64 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.15, i64 0, i64 0), i32 192, i8* getelementptr inbounds ([32 x i8], [32 x i8]* @__PRETTY_FUNCTION__.mosquitto__daemonise, i64 0, i64 0)) #13, !dbg !1024
  unreachable, !dbg !1024

; <label>:34:                                     ; preds = %29
  ret void, !dbg !1028
}

; Function Attrs: nounwind
declare i32 @fork() local_unnamed_addr #4

; Function Attrs: noreturn nounwind
declare void @exit(i32) local_unnamed_addr #7

; Function Attrs: nounwind
declare i32 @setsid() local_unnamed_addr #4

declare %struct._IO_FILE* @freopen(i8*, i8*, %struct._IO_FILE*) local_unnamed_addr #5

; Function Attrs: noreturn nounwind
declare void @__assert_fail(i8*, i8*, i32, i8*) local_unnamed_addr #7

; Function Attrs: nounwind uwtable
define i32 @main(i32, i8**) local_unnamed_addr #1 !dbg !1029 {
  %3 = alloca %struct.mosquitto__config, align 8
  %4 = alloca %struct.timeval, align 8
  call void @llvm.dbg.value(metadata i32 %0, metadata !1033, metadata !DIExpression()), !dbg !1052
  call void @llvm.dbg.value(metadata i8** %1, metadata !1034, metadata !DIExpression()), !dbg !1053
  call void @llvm.dbg.value(metadata i32* null, metadata !1035, metadata !DIExpression()), !dbg !1054
  call void @llvm.dbg.value(metadata i32 0, metadata !1036, metadata !DIExpression()), !dbg !1055
  call void @llvm.dbg.value(metadata i32 0, metadata !1037, metadata !DIExpression()), !dbg !1056
  %5 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 0, !dbg !1057
  %6 = bitcast %struct.timeval* %4 to i8*, !dbg !1058
  call void @llvm.dbg.value(metadata %struct.timeval* %4, metadata !1043, metadata !DIExpression()), !dbg !1059
  %7 = call i32 @gettimeofday(%struct.timeval* nonnull %4, %struct.timezone* null) #10, !dbg !1060
  %8 = getelementptr inbounds %struct.timeval, %struct.timeval* %4, i64 0, i32 0, !dbg !1061
  %9 = load i64, i64* %8, align 8, !dbg !1061, !tbaa !1062
  %10 = getelementptr inbounds %struct.timeval, %struct.timeval* %4, i64 0, i32 1, !dbg !1064
  %11 = load i64, i64* %10, align 8, !dbg !1064, !tbaa !1065
  %12 = add nsw i64 %11, %9, !dbg !1066
  %13 = trunc i64 %12 to i32, !dbg !1067
  tail call void @srand(i32 %13) #10, !dbg !1068
  %14 = call i8* @memset(i8* bitcast (%struct.mosquitto_db* @int_db to i8*), i32 0, i64 160), !dbg !1069
  tail call void @net__broker_init() #10, !dbg !1070
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %3, metadata !1038, metadata !DIExpression()), !dbg !1071
  call void @config__init(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto__config* nonnull %3) #10, !dbg !1072
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %3, metadata !1038, metadata !DIExpression()), !dbg !1071
  %15 = call i32 @config__parse_args(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto__config* nonnull %3, i32 %0, i8** %1) #10, !dbg !1073
  call void @llvm.dbg.value(metadata i32 %15, metadata !1042, metadata !DIExpression()), !dbg !1074
  %16 = icmp eq i32 %15, 0, !dbg !1075
  br i1 %16, label %17, label %270, !dbg !1077

; <label>:17:                                     ; preds = %2
  store %struct.mosquitto__config* %3, %struct.mosquitto__config** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 15), align 8, !dbg !1078, !tbaa !1079
  %18 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 6, !dbg !1081
  %19 = load i8, i8* %18, align 1, !dbg !1081, !tbaa !1083, !range !1084
  %20 = icmp eq i8 %19, 0, !dbg !1081
  br i1 %20, label %37, label %21, !dbg !1085

; <label>:21:                                     ; preds = %17
  call void @mosquitto__daemonise(), !dbg !1086
  %22 = load i8, i8* %18, align 1, !dbg !1088, !tbaa !1083
  %23 = icmp eq i8 %22, 0, !dbg !1088
  br i1 %23, label %37, label %24, !dbg !1090

; <label>:24:                                     ; preds = %21
  %25 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 26, !dbg !1091
  %26 = load i8*, i8** %25, align 8, !dbg !1091, !tbaa !1092
  %27 = icmp eq i8* %26, null, !dbg !1093
  br i1 %27, label %37, label %28, !dbg !1094

; <label>:28:                                     ; preds = %24
  %29 = call %struct._IO_FILE* @mosquitto__fopen(i8* nonnull %26, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.19, i64 0, i64 0), i1 zeroext false) #10, !dbg !1095
  call void @llvm.dbg.value(metadata %struct._IO_FILE* %29, metadata !1041, metadata !DIExpression()), !dbg !1097
  %30 = icmp eq %struct._IO_FILE* %29, null, !dbg !1098
  br i1 %30, label %35, label %31, !dbg !1100

; <label>:31:                                     ; preds = %28
  %32 = call i32 @getpid() #10, !dbg !1101
  %33 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* nonnull %29, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.20, i64 0, i64 0), i32 %32), !dbg !1103
  %34 = call i32 @fclose(%struct._IO_FILE* nonnull %29), !dbg !1104
  br label %37, !dbg !1105

; <label>:35:                                     ; preds = %28
  %36 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.21, i64 0, i64 0)) #10, !dbg !1106
  br label %270, !dbg !1108

; <label>:37:                                     ; preds = %17, %24, %21, %31
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %3, metadata !1038, metadata !DIExpression()), !dbg !1071
  %38 = call i32 @db__open(%struct.mosquitto__config* nonnull %3, %struct.mosquitto_db* nonnull @int_db) #10, !dbg !1109
  call void @llvm.dbg.value(metadata i32 %38, metadata !1042, metadata !DIExpression()), !dbg !1074
  %39 = icmp eq i32 %38, 0, !dbg !1110
  br i1 %39, label %42, label %40, !dbg !1112

; <label>:40:                                     ; preds = %37
  %41 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 8, i8* getelementptr inbounds ([31 x i8], [31 x i8]* @.str.22, i64 0, i64 0)) #10, !dbg !1113
  br label %270, !dbg !1115

; <label>:42:                                     ; preds = %37
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %3, metadata !1038, metadata !DIExpression()), !dbg !1071
  %43 = call i32 @log__init(%struct.mosquitto__config* nonnull %3) #10, !dbg !1116
  %44 = icmp eq i32 %43, 0, !dbg !1116
  br i1 %44, label %45, label %270, !dbg !1118

; <label>:45:                                     ; preds = %42
  %46 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 1, i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str.23, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.24, i64 0, i64 0)) #10, !dbg !1119
  %47 = load i8*, i8** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 14), align 8, !dbg !1120, !tbaa !1122
  %48 = icmp eq i8* %47, null, !dbg !1123
  br i1 %48, label %51, label %49, !dbg !1124

; <label>:49:                                     ; preds = %45
  %50 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 1, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.25, i64 0, i64 0), i8* nonnull %47) #10, !dbg !1125
  br label %53, !dbg !1127

; <label>:51:                                     ; preds = %45
  %52 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 1, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.26, i64 0, i64 0)) #10, !dbg !1128
  br label %53

; <label>:53:                                     ; preds = %51, %49
  %54 = call i32 @mosquitto_security_module_init(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1130
  call void @llvm.dbg.value(metadata i32 %54, metadata !1042, metadata !DIExpression()), !dbg !1074
  %55 = icmp eq i32 %54, 0, !dbg !1131
  br i1 %55, label %56, label %270, !dbg !1133

; <label>:56:                                     ; preds = %53
  %57 = call i32 @mosquitto_security_init(%struct.mosquitto_db* nonnull @int_db, i1 zeroext false) #10, !dbg !1134
  call void @llvm.dbg.value(metadata i32 %57, metadata !1042, metadata !DIExpression()), !dbg !1074
  %58 = icmp eq i32 %57, 0, !dbg !1135
  br i1 %58, label %59, label %270, !dbg !1137

; <label>:59:                                     ; preds = %56
  call void @sys_tree__init(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1138
  call void @llvm.dbg.value(metadata i32 0, metadata !1037, metadata !DIExpression()), !dbg !1056
  call void @llvm.dbg.value(metadata i32 0, metadata !1039, metadata !DIExpression()), !dbg !1139
  call void @llvm.dbg.value(metadata i32 0, metadata !1036, metadata !DIExpression()), !dbg !1055
  call void @llvm.dbg.value(metadata i32* null, metadata !1035, metadata !DIExpression()), !dbg !1054
  %60 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 9, !dbg !1140
  %61 = load i32, i32* %60, align 8, !dbg !1140, !tbaa !1143
  %62 = icmp sgt i32 %61, 0, !dbg !1144
  br i1 %62, label %63, label %147, !dbg !1145

; <label>:63:                                     ; preds = %59
  %64 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 8
  %65 = load %struct.mosquitto__listener*, %struct.mosquitto__listener** %64, align 8, !dbg !1146, !tbaa !1149
  br label %66, !dbg !1145

; <label>:66:                                     ; preds = %63, %136
  %67 = phi %struct.mosquitto__listener* [ %65, %63 ], [ %137, %136 ], !dbg !1146
  %68 = phi i64 [ 0, %63 ], [ %143, %136 ]
  %69 = phi i32 [ 0, %63 ], [ %142, %136 ]
  %70 = phi i32 [ 0, %63 ], [ %141, %136 ]
  %71 = phi i32* [ null, %63 ], [ %140, %136 ]
  %72 = phi i8* [ null, %63 ], [ %139, %136 ]
  %73 = phi i8* [ null, %63 ], [ %138, %136 ]
  call void @llvm.dbg.value(metadata i32* %71, metadata !1035, metadata !DIExpression()), !dbg !1054
  call void @llvm.dbg.value(metadata i32 %70, metadata !1036, metadata !DIExpression()), !dbg !1055
  call void @llvm.dbg.value(metadata i32 %69, metadata !1037, metadata !DIExpression()), !dbg !1056
  call void @llvm.dbg.value(metadata i64 %68, metadata !1039, metadata !DIExpression()), !dbg !1139
  %74 = getelementptr inbounds %struct.mosquitto__listener, %struct.mosquitto__listener* %67, i64 %68, i32 9, !dbg !1150
  %75 = load i32, i32* %74, align 8, !dbg !1150, !tbaa !1151
  %76 = icmp eq i32 %75, 0, !dbg !1152
  br i1 %76, label %77, label %136, !dbg !1153

; <label>:77:                                     ; preds = %66
  %78 = getelementptr inbounds %struct.mosquitto__listener, %struct.mosquitto__listener* %67, i64 %68, !dbg !1154
  %79 = call i32 @net__socket_listen(%struct.mosquitto__listener* %78) #10, !dbg !1155
  %80 = icmp eq i32 %79, 0, !dbg !1155
  br i1 %80, label %88, label %81, !dbg !1158

; <label>:81:                                     ; preds = %77
  %82 = call i32 @db__close(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1159
  %83 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 26, !dbg !1161
  %84 = load i8*, i8** %83, align 8, !dbg !1161, !tbaa !1092
  %85 = icmp eq i8* %84, null, !dbg !1163
  br i1 %85, label %270, label %86, !dbg !1164

; <label>:86:                                     ; preds = %81
  %87 = call i32 @remove(i8* nonnull %84) #10, !dbg !1165
  br label %270, !dbg !1167

; <label>:88:                                     ; preds = %77
  %89 = load %struct.mosquitto__listener*, %struct.mosquitto__listener** %64, align 8, !dbg !1168, !tbaa !1149
  %90 = getelementptr inbounds %struct.mosquitto__listener, %struct.mosquitto__listener* %89, i64 %68, i32 7, !dbg !1169
  %91 = load i32, i32* %90, align 8, !dbg !1169, !tbaa !1170
  %92 = add nsw i32 %91, %70, !dbg !1171
  call void @llvm.dbg.value(metadata i32 %92, metadata !1036, metadata !DIExpression()), !dbg !1055
  %93 = sext i32 %92 to i64, !dbg !1172
  %94 = shl nsw i64 %93, 2, !dbg !1173
  %95 = call i8* @mosquitto__realloc(i8* %72, i64 %94) #10, !dbg !1174
  %96 = bitcast i8* %95 to i32*, !dbg !1174
  call void @llvm.dbg.value(metadata i32* %96, metadata !1035, metadata !DIExpression()), !dbg !1054
  %97 = icmp eq i8* %95, null, !dbg !1175
  br i1 %97, label %98, label %105, !dbg !1177

; <label>:98:                                     ; preds = %88
  %99 = call i32 @db__close(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1178
  %100 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 26, !dbg !1180
  %101 = load i8*, i8** %100, align 8, !dbg !1180, !tbaa !1092
  %102 = icmp eq i8* %101, null, !dbg !1182
  br i1 %102, label %270, label %103, !dbg !1183

; <label>:103:                                    ; preds = %98
  %104 = call i32 @remove(i8* nonnull %101) #10, !dbg !1184
  br label %270, !dbg !1186

; <label>:105:                                    ; preds = %88
  call void @llvm.dbg.value(metadata i32 0, metadata !1040, metadata !DIExpression()), !dbg !1187
  call void @llvm.dbg.value(metadata i32 %69, metadata !1037, metadata !DIExpression()), !dbg !1056
  %106 = load %struct.mosquitto__listener*, %struct.mosquitto__listener** %64, align 8, !dbg !1188, !tbaa !1149
  %107 = getelementptr inbounds %struct.mosquitto__listener, %struct.mosquitto__listener* %106, i64 %68, i32 7, !dbg !1191
  %108 = load i32, i32* %107, align 8, !dbg !1191, !tbaa !1170
  %109 = icmp sgt i32 %108, 0, !dbg !1192
  br i1 %109, label %110, label %136, !dbg !1193

; <label>:110:                                    ; preds = %105
  %111 = sext i32 %69 to i64, !dbg !1193
  %112 = getelementptr inbounds %struct.mosquitto__listener, %struct.mosquitto__listener* %106, i64 %68, i32 6
  %113 = load i32*, i32** %112, align 8, !dbg !1194, !tbaa !1197
  br label %114, !dbg !1193

; <label>:114:                                    ; preds = %110, %127
  %115 = phi i64 [ %111, %110 ], [ %129, %127 ]
  %116 = phi i64 [ 0, %110 ], [ %130, %127 ]
  call void @llvm.dbg.value(metadata i64 %115, metadata !1037, metadata !DIExpression()), !dbg !1056
  call void @llvm.dbg.value(metadata i64 %116, metadata !1040, metadata !DIExpression()), !dbg !1187
  %117 = getelementptr inbounds i32, i32* %113, i64 %116, !dbg !1198
  %118 = load i32, i32* %117, align 4, !dbg !1198, !tbaa !903
  %119 = icmp eq i32 %118, -1, !dbg !1199
  br i1 %119, label %120, label %127, !dbg !1200

; <label>:120:                                    ; preds = %114
  %121 = call i32 @db__close(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1201
  %122 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 26, !dbg !1203
  %123 = load i8*, i8** %122, align 8, !dbg !1203, !tbaa !1092
  %124 = icmp eq i8* %123, null, !dbg !1205
  br i1 %124, label %270, label %125, !dbg !1206

; <label>:125:                                    ; preds = %120
  %126 = call i32 @remove(i8* nonnull %123) #10, !dbg !1207
  br label %270, !dbg !1209

; <label>:127:                                    ; preds = %114
  %128 = getelementptr inbounds i32, i32* %96, i64 %115, !dbg !1210
  store i32 %118, i32* %128, align 4, !dbg !1211, !tbaa !903
  %129 = add nsw i64 %115, 1, !dbg !1212
  %130 = add nuw nsw i64 %116, 1, !dbg !1213
  %131 = load i32, i32* %107, align 8, !dbg !1191, !tbaa !1170
  %132 = sext i32 %131 to i64, !dbg !1192
  %133 = icmp slt i64 %130, %132, !dbg !1192
  br i1 %133, label %114, label %134, !dbg !1193, !llvm.loop !1214

; <label>:134:                                    ; preds = %127
  %135 = trunc i64 %129 to i32, !dbg !1216
  br label %136, !dbg !1216

; <label>:136:                                    ; preds = %134, %105, %66
  %137 = phi %struct.mosquitto__listener* [ %67, %66 ], [ %106, %105 ], [ %106, %134 ]
  %138 = phi i8* [ %73, %66 ], [ %95, %105 ], [ %95, %134 ]
  %139 = phi i8* [ %72, %66 ], [ %95, %105 ], [ %95, %134 ]
  %140 = phi i32* [ %71, %66 ], [ %96, %105 ], [ %96, %134 ]
  %141 = phi i32 [ %70, %66 ], [ %92, %105 ], [ %92, %134 ]
  %142 = phi i32 [ %69, %66 ], [ %69, %105 ], [ %135, %134 ]
  %143 = add nuw nsw i64 %68, 1, !dbg !1216
  call void @llvm.dbg.value(metadata i32 %142, metadata !1037, metadata !DIExpression()), !dbg !1056
  call void @llvm.dbg.value(metadata i32 %141, metadata !1036, metadata !DIExpression()), !dbg !1055
  call void @llvm.dbg.value(metadata i32* %140, metadata !1035, metadata !DIExpression()), !dbg !1054
  %144 = load i32, i32* %60, align 8, !dbg !1140, !tbaa !1143
  %145 = sext i32 %144 to i64, !dbg !1144
  %146 = icmp slt i64 %143, %145, !dbg !1144
  br i1 %146, label %66, label %147, !dbg !1145, !llvm.loop !1217

; <label>:147:                                    ; preds = %136, %59
  %148 = phi i8* [ null, %59 ], [ %138, %136 ]
  %149 = phi i32* [ null, %59 ], [ %140, %136 ]
  %150 = phi i32 [ 0, %59 ], [ %141, %136 ]
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %3, metadata !1038, metadata !DIExpression()), !dbg !1071
  %151 = call i32 @drop_privileges(%struct.mosquitto__config* nonnull %3, i1 zeroext false), !dbg !1219
  call void @llvm.dbg.value(metadata i32 %151, metadata !1042, metadata !DIExpression()), !dbg !1074
  %152 = icmp eq i32 %151, 0, !dbg !1220
  br i1 %152, label %153, label %270, !dbg !1222

; <label>:153:                                    ; preds = %147
  %154 = call void (i32)* @signal(i32 2, void (i32)* nonnull @handle_sigint) #10, !dbg !1223
  %155 = call void (i32)* @signal(i32 15, void (i32)* nonnull @handle_sigint) #10, !dbg !1224
  %156 = call void (i32)* @signal(i32 1, void (i32)* nonnull @handle_sighup) #10, !dbg !1225
  %157 = call void (i32)* @signal(i32 10, void (i32)* nonnull @handle_sigusr1) #10, !dbg !1226
  %158 = call void (i32)* @signal(i32 12, void (i32)* nonnull @handle_sigusr2) #10, !dbg !1227
  %159 = call void (i32)* @signal(i32 13, void (i32)* inttoptr (i64 1 to void (i32)*)) #10, !dbg !1228
  call void @llvm.dbg.value(metadata i32 0, metadata !1039, metadata !DIExpression()), !dbg !1139
  %160 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 35, !dbg !1229
  %161 = load i32, i32* %160, align 8, !dbg !1229, !tbaa !1232
  %162 = icmp sgt i32 %161, 0, !dbg !1233
  br i1 %162, label %163, label %181, !dbg !1234

; <label>:163:                                    ; preds = %153
  %164 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 34
  br label %165, !dbg !1234

; <label>:165:                                    ; preds = %163, %176
  %166 = phi i64 [ 0, %163 ], [ %177, %176 ]
  call void @llvm.dbg.value(metadata i64 %166, metadata !1039, metadata !DIExpression()), !dbg !1139
  %167 = load %struct.mosquitto__bridge*, %struct.mosquitto__bridge** %164, align 8, !dbg !1235, !tbaa !1238
  %168 = getelementptr inbounds %struct.mosquitto__bridge, %struct.mosquitto__bridge* %167, i64 %166, !dbg !1239
  %169 = call i32 @bridge__new(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto__bridge* %168) #10, !dbg !1240
  %170 = icmp eq i32 %169, 0, !dbg !1240
  br i1 %170, label %176, label %171, !dbg !1241

; <label>:171:                                    ; preds = %165
  %172 = load %struct.mosquitto__bridge*, %struct.mosquitto__bridge** %164, align 8, !dbg !1242, !tbaa !1238
  %173 = getelementptr inbounds %struct.mosquitto__bridge, %struct.mosquitto__bridge* %172, i64 %166, i32 0, !dbg !1244
  %174 = load i8*, i8** %173, align 8, !dbg !1244, !tbaa !1245
  %175 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 4, i8* getelementptr inbounds ([41 x i8], [41 x i8]* @.str.27, i64 0, i64 0), i8* %174) #10, !dbg !1247
  br label %176, !dbg !1248

; <label>:176:                                    ; preds = %165, %171
  %177 = add nuw nsw i64 %166, 1, !dbg !1249
  %178 = load i32, i32* %160, align 8, !dbg !1229, !tbaa !1232
  %179 = sext i32 %178 to i64, !dbg !1233
  %180 = icmp slt i64 %177, %179, !dbg !1233
  br i1 %180, label %165, label %181, !dbg !1234, !llvm.loop !1250

; <label>:181:                                    ; preds = %176, %153
  store i32 1, i32* @run, align 4, !dbg !1252, !tbaa !903
  %182 = call i32 @mosquitto_main_loop(%struct.mosquitto_db* nonnull @int_db, i32* %149, i32 %150) #10, !dbg !1253
  call void @llvm.dbg.value(metadata i32 %182, metadata !1042, metadata !DIExpression()), !dbg !1074
  %183 = call i32 (%struct.mosquitto*, i32, i8*, ...) @log__printf(%struct.mosquitto* null, i32 1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.28, i64 0, i64 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.24, i64 0, i64 0)) #10, !dbg !1254
  %184 = load %struct.mosquitto*, %struct.mosquitto** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 4), align 8, !dbg !1255, !tbaa !1257
  call void @llvm.dbg.value(metadata %struct.mosquitto* %184, metadata !1050, metadata !DIExpression()), !dbg !1258
  %185 = icmp eq %struct.mosquitto* %184, null, !dbg !1255
  br i1 %185, label %193, label %186, !dbg !1255

; <label>:186:                                    ; preds = %181
  br label %187, !dbg !1255

; <label>:187:                                    ; preds = %186, %187
  %188 = phi %struct.mosquitto* [ %191, %187 ], [ %184, %186 ]
  %189 = getelementptr inbounds %struct.mosquitto, %struct.mosquitto* %188, i64 0, i32 64, i32 2, !dbg !1255
  %190 = bitcast i8** %189 to %struct.mosquitto**, !dbg !1255
  %191 = load %struct.mosquitto*, %struct.mosquitto** %190, align 8, !dbg !1255, !tbaa !1259
  call void @llvm.dbg.value(metadata %struct.mosquitto* %188, metadata !1050, metadata !DIExpression()), !dbg !1258
  call void @llvm.dbg.value(metadata %struct.mosquitto* %191, metadata !1051, metadata !DIExpression()), !dbg !1264
  call void @context__send_will(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto* nonnull %188) #10, !dbg !1265
  %192 = icmp eq %struct.mosquitto* %191, null, !dbg !1268
  call void @llvm.dbg.value(metadata %struct.mosquitto* %191, metadata !1051, metadata !DIExpression(DW_OP_plus_uconst, 512, DW_OP_deref, DW_OP_stack_value)), !dbg !1264
  call void @llvm.dbg.value(metadata %struct.mosquitto* %191, metadata !1050, metadata !DIExpression()), !dbg !1258
  br i1 %192, label %193, label %187, !dbg !1268

; <label>:193:                                    ; preds = %187, %181
  call void @will_delay__send_all(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1269
  %194 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 21, !dbg !1270
  %195 = load i8, i8* %194, align 4, !dbg !1270, !tbaa !1272, !range !1084
  %196 = icmp eq i8 %195, 0, !dbg !1270
  br i1 %196, label %199, label %197, !dbg !1273

; <label>:197:                                    ; preds = %193
  %198 = call i32 @persist__backup(%struct.mosquitto_db* nonnull @int_db, i1 zeroext true) #10, !dbg !1274
  br label %199, !dbg !1276

; <label>:199:                                    ; preds = %193, %197
  call void @session_expiry__remove_all(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1277
  %200 = load %struct.mosquitto*, %struct.mosquitto** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 4), align 8, !dbg !1278, !tbaa !1257
  call void @llvm.dbg.value(metadata %struct.mosquitto* %200, metadata !1050, metadata !DIExpression()), !dbg !1258
  %201 = icmp eq %struct.mosquitto* %200, null, !dbg !1278
  br i1 %201, label %209, label %202, !dbg !1278

; <label>:202:                                    ; preds = %199
  br label %203, !dbg !1278

; <label>:203:                                    ; preds = %202, %203
  %204 = phi %struct.mosquitto* [ %207, %203 ], [ %200, %202 ]
  %205 = getelementptr inbounds %struct.mosquitto, %struct.mosquitto* %204, i64 0, i32 64, i32 2, !dbg !1278
  %206 = bitcast i8** %205 to %struct.mosquitto**, !dbg !1278
  %207 = load %struct.mosquitto*, %struct.mosquitto** %206, align 8, !dbg !1278, !tbaa !1259
  call void @llvm.dbg.value(metadata %struct.mosquitto* %204, metadata !1050, metadata !DIExpression()), !dbg !1258
  call void @llvm.dbg.value(metadata %struct.mosquitto* %207, metadata !1051, metadata !DIExpression()), !dbg !1264
  call void @context__cleanup(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto* nonnull %204, i1 zeroext true) #10, !dbg !1280
  %208 = icmp eq %struct.mosquitto* %207, null, !dbg !1283
  call void @llvm.dbg.value(metadata %struct.mosquitto* %207, metadata !1051, metadata !DIExpression(DW_OP_plus_uconst, 512, DW_OP_deref, DW_OP_stack_value)), !dbg !1264
  call void @llvm.dbg.value(metadata %struct.mosquitto* %207, metadata !1050, metadata !DIExpression()), !dbg !1258
  br i1 %208, label %209, label %203, !dbg !1283

; <label>:209:                                    ; preds = %203, %199
  %210 = load %struct.mosquitto*, %struct.mosquitto** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 5), align 8, !dbg !1284, !tbaa !1286
  call void @llvm.dbg.value(metadata %struct.mosquitto* %210, metadata !1050, metadata !DIExpression()), !dbg !1258
  %211 = icmp eq %struct.mosquitto* %210, null, !dbg !1284
  br i1 %211, label %219, label %212, !dbg !1284

; <label>:212:                                    ; preds = %209
  br label %213, !dbg !1284

; <label>:213:                                    ; preds = %212, %213
  %214 = phi %struct.mosquitto* [ %217, %213 ], [ %210, %212 ]
  %215 = getelementptr inbounds %struct.mosquitto, %struct.mosquitto* %214, i64 0, i32 65, i32 2, !dbg !1284
  %216 = bitcast i8** %215 to %struct.mosquitto**, !dbg !1284
  %217 = load %struct.mosquitto*, %struct.mosquitto** %216, align 8, !dbg !1284, !tbaa !1287
  call void @llvm.dbg.value(metadata %struct.mosquitto* %214, metadata !1050, metadata !DIExpression()), !dbg !1258
  call void @llvm.dbg.value(metadata %struct.mosquitto* %217, metadata !1051, metadata !DIExpression()), !dbg !1264
  call void @context__cleanup(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto* nonnull %214, i1 zeroext true) #10, !dbg !1288
  %218 = icmp eq %struct.mosquitto* %217, null, !dbg !1291
  call void @llvm.dbg.value(metadata %struct.mosquitto* %217, metadata !1051, metadata !DIExpression(DW_OP_plus_uconst, 568, DW_OP_deref, DW_OP_stack_value)), !dbg !1264
  call void @llvm.dbg.value(metadata %struct.mosquitto* %217, metadata !1050, metadata !DIExpression()), !dbg !1258
  br i1 %218, label %219, label %213, !dbg !1291

; <label>:219:                                    ; preds = %213, %209
  call void @llvm.dbg.value(metadata i32 0, metadata !1039, metadata !DIExpression()), !dbg !1139
  %220 = load i32, i32* getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 11), align 8, !dbg !1292, !tbaa !1295
  %221 = icmp sgt i32 %220, 0, !dbg !1296
  %222 = load %struct.mosquitto**, %struct.mosquitto*** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 7), align 8, !tbaa !1297
  br i1 %221, label %223, label %240, !dbg !1298

; <label>:223:                                    ; preds = %219
  br label %224, !dbg !1299

; <label>:224:                                    ; preds = %223, %234
  %225 = phi i32 [ %236, %234 ], [ %220, %223 ]
  %226 = phi i64 [ %237, %234 ], [ 0, %223 ]
  %227 = phi %struct.mosquitto** [ %235, %234 ], [ %222, %223 ]
  call void @llvm.dbg.value(metadata i64 %226, metadata !1039, metadata !DIExpression()), !dbg !1139
  %228 = getelementptr inbounds %struct.mosquitto*, %struct.mosquitto** %227, i64 %226, !dbg !1299
  %229 = load %struct.mosquitto*, %struct.mosquitto** %228, align 8, !dbg !1299, !tbaa !1018
  %230 = icmp eq %struct.mosquitto* %229, null, !dbg !1299
  br i1 %230, label %234, label %231, !dbg !1302

; <label>:231:                                    ; preds = %224
  call void @context__cleanup(%struct.mosquitto_db* nonnull @int_db, %struct.mosquitto* nonnull %229, i1 zeroext true) #10, !dbg !1303
  %232 = load i32, i32* getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 11), align 8, !dbg !1292, !tbaa !1295
  %233 = load %struct.mosquitto**, %struct.mosquitto*** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 7), align 8, !tbaa !1297
  br label %234, !dbg !1305

; <label>:234:                                    ; preds = %224, %231
  %235 = phi %struct.mosquitto** [ %227, %224 ], [ %233, %231 ]
  %236 = phi i32 [ %225, %224 ], [ %232, %231 ], !dbg !1292
  %237 = add nuw nsw i64 %226, 1, !dbg !1306
  %238 = sext i32 %236 to i64, !dbg !1296
  %239 = icmp slt i64 %237, %238, !dbg !1296
  br i1 %239, label %224, label %240, !dbg !1298, !llvm.loop !1307

; <label>:240:                                    ; preds = %234, %219
  %241 = phi %struct.mosquitto** [ %222, %219 ], [ %235, %234 ]
  %242 = bitcast %struct.mosquitto** %241 to i8*, !dbg !1309
  call void @mosquitto__free(i8* %242) #10, !dbg !1310
  call void @context__free_disused(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1311
  %243 = call i32 @db__close(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1312
  %244 = icmp eq i32* %149, null, !dbg !1313
  br i1 %244, label %260, label %245, !dbg !1315

; <label>:245:                                    ; preds = %240
  call void @llvm.dbg.value(metadata i32 0, metadata !1039, metadata !DIExpression()), !dbg !1139
  %246 = icmp sgt i32 %150, 0, !dbg !1316
  br i1 %246, label %247, label %259, !dbg !1320

; <label>:247:                                    ; preds = %245
  %248 = zext i32 %150 to i64
  br label %249, !dbg !1320

; <label>:249:                                    ; preds = %256, %247
  %250 = phi i64 [ 0, %247 ], [ %257, %256 ]
  call void @llvm.dbg.value(metadata i64 %250, metadata !1039, metadata !DIExpression()), !dbg !1139
  %251 = getelementptr inbounds i32, i32* %149, i64 %250, !dbg !1321
  %252 = load i32, i32* %251, align 4, !dbg !1321, !tbaa !903
  %253 = icmp eq i32 %252, -1, !dbg !1324
  br i1 %253, label %256, label %254, !dbg !1325

; <label>:254:                                    ; preds = %249
  %255 = call i32 @close(i32 %252) #10, !dbg !1326
  br label %256, !dbg !1328

; <label>:256:                                    ; preds = %249, %254
  %257 = add nuw nsw i64 %250, 1, !dbg !1329
  %258 = icmp eq i64 %257, %248, !dbg !1316
  br i1 %258, label %259, label %249, !dbg !1320, !llvm.loop !1330

; <label>:259:                                    ; preds = %256, %245
  call void @mosquitto__free(i8* %148) #10, !dbg !1332
  br label %260, !dbg !1333

; <label>:260:                                    ; preds = %240, %259
  %261 = call i32 @mosquitto_security_module_cleanup(%struct.mosquitto_db* nonnull @int_db) #10, !dbg !1334
  %262 = getelementptr inbounds %struct.mosquitto__config, %struct.mosquitto__config* %3, i64 0, i32 26, !dbg !1335
  %263 = load i8*, i8** %262, align 8, !dbg !1335, !tbaa !1092
  %264 = icmp eq i8* %263, null, !dbg !1337
  br i1 %264, label %267, label %265, !dbg !1338

; <label>:265:                                    ; preds = %260
  %266 = call i32 @remove(i8* nonnull %263) #10, !dbg !1339
  br label %267, !dbg !1341

; <label>:267:                                    ; preds = %260, %265
  call void @llvm.dbg.value(metadata %struct.mosquitto__config* %3, metadata !1038, metadata !DIExpression()), !dbg !1071
  %268 = call i32 @log__close(%struct.mosquitto__config* nonnull %3) #10, !dbg !1342
  %269 = load %struct.mosquitto__config*, %struct.mosquitto__config** getelementptr inbounds (%struct.mosquitto_db, %struct.mosquitto_db* @int_db, i64 0, i32 15), align 8, !dbg !1343, !tbaa !1079
  call void @config__cleanup(%struct.mosquitto__config* %269) #10, !dbg !1344
  call void @net__broker_cleanup() #10, !dbg !1345
  br label %270, !dbg !1346

; <label>:270:                                    ; preds = %147, %125, %120, %103, %98, %86, %81, %56, %53, %42, %2, %267, %40, %35
  %271 = phi i32 [ %38, %40 ], [ %182, %267 ], [ 1, %35 ], [ %15, %2 ], [ 1, %42 ], [ %54, %53 ], [ %57, %56 ], [ 1, %81 ], [ 1, %86 ], [ 1, %98 ], [ 1, %103 ], [ 1, %120 ], [ 1, %125 ], [ %151, %147 ]
  ret i32 %271, !dbg !1347
}

; Function Attrs: nounwind
declare i32 @gettimeofday(%struct.timeval* nocapture, %struct.timezone* nocapture) local_unnamed_addr #4

; Function Attrs: nounwind
declare void @srand(i32) local_unnamed_addr #4

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2

declare void @net__broker_init() local_unnamed_addr #5

declare void @config__init(%struct.mosquitto_db*, %struct.mosquitto__config*) local_unnamed_addr #5

declare i32 @config__parse_args(%struct.mosquitto_db*, %struct.mosquitto__config*, i32, i8**) local_unnamed_addr #5

declare %struct._IO_FILE* @mosquitto__fopen(i8*, i8*, i1 zeroext) local_unnamed_addr #5

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...) local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @getpid() local_unnamed_addr #4

; Function Attrs: nounwind
declare i32 @fclose(%struct._IO_FILE* nocapture) local_unnamed_addr #4

declare i32 @db__open(%struct.mosquitto__config*, %struct.mosquitto_db*) local_unnamed_addr #5

declare i32 @log__init(%struct.mosquitto__config*) local_unnamed_addr #5

declare i32 @mosquitto_security_module_init(%struct.mosquitto_db*) local_unnamed_addr #5

declare i32 @mosquitto_security_init(%struct.mosquitto_db*, i1 zeroext) local_unnamed_addr #5

declare void @sys_tree__init(%struct.mosquitto_db*) local_unnamed_addr #5

declare i32 @net__socket_listen(%struct.mosquitto__listener*) local_unnamed_addr #5

declare i32 @db__close(%struct.mosquitto_db*) local_unnamed_addr #5

; Function Attrs: nounwind
declare i32 @remove(i8* nocapture readonly) local_unnamed_addr #4

declare i8* @mosquitto__realloc(i8*, i64) local_unnamed_addr #5

; Function Attrs: nounwind
declare void (i32)* @signal(i32, void (i32)*) local_unnamed_addr #4

declare void @handle_sigint(i32) #5

declare void @handle_sighup(i32) #5

declare void @handle_sigusr1(i32) #5

declare void @handle_sigusr2(i32) #5

declare i32 @bridge__new(%struct.mosquitto_db*, %struct.mosquitto__bridge*) local_unnamed_addr #5

declare i32 @mosquitto_main_loop(%struct.mosquitto_db*, i32*, i32) local_unnamed_addr #5

declare void @context__send_will(%struct.mosquitto_db*, %struct.mosquitto*) local_unnamed_addr #5

declare void @will_delay__send_all(%struct.mosquitto_db*) local_unnamed_addr #5

declare i32 @persist__backup(%struct.mosquitto_db*, i1 zeroext) local_unnamed_addr #5

declare void @session_expiry__remove_all(%struct.mosquitto_db*) local_unnamed_addr #5

declare void @context__cleanup(%struct.mosquitto_db*, %struct.mosquitto*, i1 zeroext) local_unnamed_addr #5

declare void @mosquitto__free(i8*) local_unnamed_addr #5

declare void @context__free_disused(%struct.mosquitto_db*) local_unnamed_addr #5

declare i32 @close(i32) local_unnamed_addr #5

declare i32 @mosquitto_security_module_cleanup(%struct.mosquitto_db*) local_unnamed_addr #5

declare i32 @log__close(%struct.mosquitto__config*) local_unnamed_addr #5

declare void @config__cleanup(%struct.mosquitto__config*) local_unnamed_addr #5

declare void @net__broker_cleanup() local_unnamed_addr #5

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #8

; Function Attrs: noinline nounwind uwtable
define i8* @memset(i8*, i32, i64) #9 !dbg !1348 {
  %4 = alloca i8*, align 8
  %5 = alloca i32, align 4
  %6 = alloca i64, align 8
  %7 = alloca i8*, align 8
  store i8* %0, i8** %4, align 8
  call void @llvm.dbg.declare(metadata i8** %4, metadata !1353, metadata !DIExpression()), !dbg !1354
  store i32 %1, i32* %5, align 4
  call void @llvm.dbg.declare(metadata i32* %5, metadata !1355, metadata !DIExpression()), !dbg !1356
  store i64 %2, i64* %6, align 8
  call void @llvm.dbg.declare(metadata i64* %6, metadata !1357, metadata !DIExpression()), !dbg !1358
  call void @llvm.dbg.declare(metadata i8** %7, metadata !1359, metadata !DIExpression()), !dbg !1360
  %8 = load i8*, i8** %4, align 8, !dbg !1361
  store i8* %8, i8** %7, align 8, !dbg !1360
  br label %9, !dbg !1362

; <label>:9:                                      ; preds = %13, %3
  %10 = load i64, i64* %6, align 8, !dbg !1363
  %11 = add i64 %10, -1, !dbg !1363
  store i64 %11, i64* %6, align 8, !dbg !1363
  %12 = icmp ugt i64 %10, 0, !dbg !1364
  br i1 %12, label %13, label %18, !dbg !1362

; <label>:13:                                     ; preds = %9
  %14 = load i32, i32* %5, align 4, !dbg !1365
  %15 = trunc i32 %14 to i8, !dbg !1365
  %16 = load i8*, i8** %7, align 8, !dbg !1366
  %17 = getelementptr inbounds i8, i8* %16, i32 1, !dbg !1366
  store i8* %17, i8** %7, align 8, !dbg !1366
  store i8 %15, i8* %16, align 1, !dbg !1367
  br label %9, !dbg !1362, !llvm.loop !1368

; <label>:18:                                     ; preds = %9
  %19 = load i8*, i8** %4, align 8, !dbg !1369
  ret i8* %19, !dbg !1370
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #8

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nounwind readnone speculatable }
attributes #9 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #10 = { nounwind }
attributes #11 = { nounwind readonly }
attributes #12 = { nounwind readnone }
attributes #13 = { noreturn nounwind }

!llvm.dbg.cu = !{!2, !819}
!llvm.module.flags = !{!822, !823, !824}
!llvm.ident = !{!825, !825}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "flag_reload", scope: !2, file: !3, line: 58, type: !184, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, retainedTypes: !110, globals: !682)
!3 = !DIFile(filename: "mosquitto.c", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!4 = !{!5, !13, !36, !50, !54, !60, !66, !72, !76}
!5 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto__protocol", file: !6, line: 117, size: 32, elements: !7)
!6 = !DIFile(filename: "../lib/mosquitto_internal.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!7 = !{!8, !9, !10, !11, !12}
!8 = !DIEnumerator(name: "mosq_p_invalid", value: 0)
!9 = !DIEnumerator(name: "mosq_p_mqtt31", value: 1)
!10 = !DIEnumerator(name: "mosq_p_mqtt311", value: 2)
!11 = !DIEnumerator(name: "mosq_p_mqtts", value: 3)
!12 = !DIEnumerator(name: "mosq_p_mqtt5", value: 5)
!13 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto_client_state", file: !6, line: 93, size: 32, elements: !14)
!14 = !{!15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35}
!15 = !DIEnumerator(name: "mosq_cs_new", value: 0)
!16 = !DIEnumerator(name: "mosq_cs_connected", value: 1)
!17 = !DIEnumerator(name: "mosq_cs_disconnecting", value: 2)
!18 = !DIEnumerator(name: "mosq_cs_active", value: 3)
!19 = !DIEnumerator(name: "mosq_cs_connect_pending", value: 4)
!20 = !DIEnumerator(name: "mosq_cs_connect_srv", value: 5)
!21 = !DIEnumerator(name: "mosq_cs_disconnect_ws", value: 6)
!22 = !DIEnumerator(name: "mosq_cs_disconnected", value: 7)
!23 = !DIEnumerator(name: "mosq_cs_socks5_new", value: 8)
!24 = !DIEnumerator(name: "mosq_cs_socks5_start", value: 9)
!25 = !DIEnumerator(name: "mosq_cs_socks5_request", value: 10)
!26 = !DIEnumerator(name: "mosq_cs_socks5_reply", value: 11)
!27 = !DIEnumerator(name: "mosq_cs_socks5_auth_ok", value: 12)
!28 = !DIEnumerator(name: "mosq_cs_socks5_userpass_reply", value: 13)
!29 = !DIEnumerator(name: "mosq_cs_socks5_send_userpass", value: 14)
!30 = !DIEnumerator(name: "mosq_cs_expiring", value: 15)
!31 = !DIEnumerator(name: "mosq_cs_duplicate", value: 17)
!32 = !DIEnumerator(name: "mosq_cs_disconnect_with_will", value: 18)
!33 = !DIEnumerator(name: "mosq_cs_disused", value: 19)
!34 = !DIEnumerator(name: "mosq_cs_authenticating", value: 20)
!35 = !DIEnumerator(name: "mosq_cs_reauthenticating", value: 21)
!36 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto_msg_state", file: !6, line: 78, size: 32, elements: !37)
!37 = !{!38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49}
!38 = !DIEnumerator(name: "mosq_ms_invalid", value: 0)
!39 = !DIEnumerator(name: "mosq_ms_publish_qos0", value: 1)
!40 = !DIEnumerator(name: "mosq_ms_publish_qos1", value: 2)
!41 = !DIEnumerator(name: "mosq_ms_wait_for_puback", value: 3)
!42 = !DIEnumerator(name: "mosq_ms_publish_qos2", value: 4)
!43 = !DIEnumerator(name: "mosq_ms_wait_for_pubrec", value: 5)
!44 = !DIEnumerator(name: "mosq_ms_resend_pubrel", value: 6)
!45 = !DIEnumerator(name: "mosq_ms_wait_for_pubrel", value: 7)
!46 = !DIEnumerator(name: "mosq_ms_resend_pubcomp", value: 8)
!47 = !DIEnumerator(name: "mosq_ms_wait_for_pubcomp", value: 9)
!48 = !DIEnumerator(name: "mosq_ms_send_pubrec", value: 10)
!49 = !DIEnumerator(name: "mosq_ms_queued", value: 11)
!50 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto__keyform", file: !6, line: 176, size: 32, elements: !51)
!51 = !{!52, !53}
!52 = !DIEnumerator(name: "mosq_k_pem", value: 0)
!53 = !DIEnumerator(name: "mosq_k_engine", value: 1)
!54 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto__bridge_direction", file: !55, line: 456, size: 32, elements: !56)
!55 = !DIFile(filename: "./mosquitto_broker_internal.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!56 = !{!57, !58, !59}
!57 = !DIEnumerator(name: "bd_out", value: 0)
!58 = !DIEnumerator(name: "bd_in", value: 1)
!59 = !DIEnumerator(name: "bd_both", value: 2)
!60 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto_bridge_start_type", file: !55, line: 462, size: 32, elements: !61)
!61 = !{!62, !63, !64, !65}
!62 = !DIEnumerator(name: "bst_automatic", value: 0)
!63 = !DIEnumerator(name: "bst_lazy", value: 1)
!64 = !DIEnumerator(name: "bst_manual", value: 2)
!65 = !DIEnumerator(name: "bst_once", value: 3)
!66 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto_protocol", file: !67, line: 28, size: 32, elements: !68)
!67 = !DIFile(filename: "./mosquitto_broker.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!68 = !{!69, !70, !71}
!69 = !DIEnumerator(name: "mp_mqtt", value: 0)
!70 = !DIEnumerator(name: "mp_mqttsn", value: 1)
!71 = !DIEnumerator(name: "mp_websockets", value: 2)
!72 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosquitto_msg_direction", file: !6, line: 73, size: 32, elements: !73)
!73 = !{!74, !75}
!74 = !DIEnumerator(name: "mosq_md_in", value: 0)
!75 = !DIEnumerator(name: "mosq_md_out", value: 1)
!76 = !DICompositeType(tag: DW_TAG_enumeration_type, name: "mosq_err_t", file: !77, line: 69, size: 32, elements: !78)
!77 = !DIFile(filename: "../lib/mosquitto.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!78 = !{!79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109}
!79 = !DIEnumerator(name: "MOSQ_ERR_AUTH_CONTINUE", value: -4)
!80 = !DIEnumerator(name: "MOSQ_ERR_NO_SUBSCRIBERS", value: -3)
!81 = !DIEnumerator(name: "MOSQ_ERR_SUB_EXISTS", value: -2)
!82 = !DIEnumerator(name: "MOSQ_ERR_CONN_PENDING", value: -1)
!83 = !DIEnumerator(name: "MOSQ_ERR_SUCCESS", value: 0)
!84 = !DIEnumerator(name: "MOSQ_ERR_NOMEM", value: 1)
!85 = !DIEnumerator(name: "MOSQ_ERR_PROTOCOL", value: 2)
!86 = !DIEnumerator(name: "MOSQ_ERR_INVAL", value: 3)
!87 = !DIEnumerator(name: "MOSQ_ERR_NO_CONN", value: 4)
!88 = !DIEnumerator(name: "MOSQ_ERR_CONN_REFUSED", value: 5)
!89 = !DIEnumerator(name: "MOSQ_ERR_NOT_FOUND", value: 6)
!90 = !DIEnumerator(name: "MOSQ_ERR_CONN_LOST", value: 7)
!91 = !DIEnumerator(name: "MOSQ_ERR_TLS", value: 8)
!92 = !DIEnumerator(name: "MOSQ_ERR_PAYLOAD_SIZE", value: 9)
!93 = !DIEnumerator(name: "MOSQ_ERR_NOT_SUPPORTED", value: 10)
!94 = !DIEnumerator(name: "MOSQ_ERR_AUTH", value: 11)
!95 = !DIEnumerator(name: "MOSQ_ERR_ACL_DENIED", value: 12)
!96 = !DIEnumerator(name: "MOSQ_ERR_UNKNOWN", value: 13)
!97 = !DIEnumerator(name: "MOSQ_ERR_ERRNO", value: 14)
!98 = !DIEnumerator(name: "MOSQ_ERR_EAI", value: 15)
!99 = !DIEnumerator(name: "MOSQ_ERR_PROXY", value: 16)
!100 = !DIEnumerator(name: "MOSQ_ERR_PLUGIN_DEFER", value: 17)
!101 = !DIEnumerator(name: "MOSQ_ERR_MALFORMED_UTF8", value: 18)
!102 = !DIEnumerator(name: "MOSQ_ERR_KEEPALIVE", value: 19)
!103 = !DIEnumerator(name: "MOSQ_ERR_LOOKUP", value: 20)
!104 = !DIEnumerator(name: "MOSQ_ERR_MALFORMED_PACKET", value: 21)
!105 = !DIEnumerator(name: "MOSQ_ERR_DUPLICATE_PROPERTY", value: 22)
!106 = !DIEnumerator(name: "MOSQ_ERR_TLS_HANDSHAKE", value: 23)
!107 = !DIEnumerator(name: "MOSQ_ERR_QOS_NOT_SUPPORTED", value: 24)
!108 = !DIEnumerator(name: "MOSQ_ERR_OVERSIZE_PACKET", value: 25)
!109 = !DIEnumerator(name: "MOSQ_ERR_OCSP", value: 26)
!110 = !{!111, !117, !191}
!111 = !DIDerivedType(tag: DW_TAG_typedef, name: "__sighandler_t", file: !112, line: 72, baseType: !113)
!112 = !DIFile(filename: "/usr/include/signal.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!113 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !114, size: 64)
!114 = !DISubroutineType(types: !115)
!115 = !{null, !116}
!116 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!117 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !118, size: 64)
!118 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto", file: !6, line: 208, size: 5056, elements: !119)
!119 = !{!120, !122, !123, !126, !127, !128, !129, !135, !136, !137, !142, !143, !144, !169, !170, !171, !196, !202, !209, !210, !211, !212, !213, !218, !222, !223, !224, !225, !226, !230, !231, !232, !233, !234, !235, !236, !237, !238, !239, !240, !241, !242, !243, !244, !245, !246, !247, !248, !249, !250, !313, !624, !625, !626, !627, !628, !657, !664, !665, !666, !667, !668, !669, !670, !671, !672, !673, !674, !681}
!120 = !DIDerivedType(tag: DW_TAG_member, name: "sock", scope: !118, file: !6, line: 209, baseType: !121, size: 32)
!121 = !DIDerivedType(tag: DW_TAG_typedef, name: "mosq_sock_t", file: !6, line: 70, baseType: !116)
!122 = !DIDerivedType(tag: DW_TAG_member, name: "protocol", scope: !118, file: !6, line: 216, baseType: !5, size: 32, offset: 32)
!123 = !DIDerivedType(tag: DW_TAG_member, name: "address", scope: !118, file: !6, line: 217, baseType: !124, size: 64, offset: 64)
!124 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !125, size: 64)
!125 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!126 = !DIDerivedType(tag: DW_TAG_member, name: "id", scope: !118, file: !6, line: 218, baseType: !124, size: 64, offset: 128)
!127 = !DIDerivedType(tag: DW_TAG_member, name: "username", scope: !118, file: !6, line: 219, baseType: !124, size: 64, offset: 192)
!128 = !DIDerivedType(tag: DW_TAG_member, name: "password", scope: !118, file: !6, line: 220, baseType: !124, size: 64, offset: 256)
!129 = !DIDerivedType(tag: DW_TAG_member, name: "keepalive", scope: !118, file: !6, line: 221, baseType: !130, size: 16, offset: 320)
!130 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint16_t", file: !131, line: 25, baseType: !132)
!131 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/stdint-uintn.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!132 = !DIDerivedType(tag: DW_TAG_typedef, name: "__uint16_t", file: !133, line: 39, baseType: !134)
!133 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/types.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!134 = !DIBasicType(name: "unsigned short", size: 16, encoding: DW_ATE_unsigned)
!135 = !DIDerivedType(tag: DW_TAG_member, name: "last_mid", scope: !118, file: !6, line: 222, baseType: !130, size: 16, offset: 336)
!136 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !118, file: !6, line: 223, baseType: !13, size: 32, offset: 352)
!137 = !DIDerivedType(tag: DW_TAG_member, name: "last_msg_in", scope: !118, file: !6, line: 224, baseType: !138, size: 64, offset: 384)
!138 = !DIDerivedType(tag: DW_TAG_typedef, name: "time_t", file: !139, line: 7, baseType: !140)
!139 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/types/time_t.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!140 = !DIDerivedType(tag: DW_TAG_typedef, name: "__time_t", file: !133, line: 148, baseType: !141)
!141 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)
!142 = !DIDerivedType(tag: DW_TAG_member, name: "next_msg_out", scope: !118, file: !6, line: 225, baseType: !138, size: 64, offset: 448)
!143 = !DIDerivedType(tag: DW_TAG_member, name: "ping_t", scope: !118, file: !6, line: 226, baseType: !138, size: 64, offset: 512)
!144 = !DIDerivedType(tag: DW_TAG_member, name: "in_packet", scope: !118, file: !6, line: 227, baseType: !145, size: 320, offset: 576)
!145 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__packet", file: !6, line: 150, size: 320, elements: !146)
!146 = !{!147, !152, !154, !158, !159, !160, !161, !162, !163, !164}
!147 = !DIDerivedType(tag: DW_TAG_member, name: "payload", scope: !145, file: !6, line: 151, baseType: !148, size: 64)
!148 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !149, size: 64)
!149 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint8_t", file: !131, line: 24, baseType: !150)
!150 = !DIDerivedType(tag: DW_TAG_typedef, name: "__uint8_t", file: !133, line: 37, baseType: !151)
!151 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!152 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !145, file: !6, line: 152, baseType: !153, size: 64, offset: 64)
!153 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !145, size: 64)
!154 = !DIDerivedType(tag: DW_TAG_member, name: "remaining_mult", scope: !145, file: !6, line: 153, baseType: !155, size: 32, offset: 128)
!155 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint32_t", file: !131, line: 26, baseType: !156)
!156 = !DIDerivedType(tag: DW_TAG_typedef, name: "__uint32_t", file: !133, line: 41, baseType: !157)
!157 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!158 = !DIDerivedType(tag: DW_TAG_member, name: "remaining_length", scope: !145, file: !6, line: 154, baseType: !155, size: 32, offset: 160)
!159 = !DIDerivedType(tag: DW_TAG_member, name: "packet_length", scope: !145, file: !6, line: 155, baseType: !155, size: 32, offset: 192)
!160 = !DIDerivedType(tag: DW_TAG_member, name: "to_process", scope: !145, file: !6, line: 156, baseType: !155, size: 32, offset: 224)
!161 = !DIDerivedType(tag: DW_TAG_member, name: "pos", scope: !145, file: !6, line: 157, baseType: !155, size: 32, offset: 256)
!162 = !DIDerivedType(tag: DW_TAG_member, name: "mid", scope: !145, file: !6, line: 158, baseType: !130, size: 16, offset: 288)
!163 = !DIDerivedType(tag: DW_TAG_member, name: "command", scope: !145, file: !6, line: 159, baseType: !149, size: 8, offset: 304)
!164 = !DIDerivedType(tag: DW_TAG_member, name: "remaining_count", scope: !145, file: !6, line: 160, baseType: !165, size: 8, offset: 312)
!165 = !DIDerivedType(tag: DW_TAG_typedef, name: "int8_t", file: !166, line: 24, baseType: !167)
!166 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/stdint-intn.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!167 = !DIDerivedType(tag: DW_TAG_typedef, name: "__int8_t", file: !133, line: 36, baseType: !168)
!168 = !DIBasicType(name: "signed char", size: 8, encoding: DW_ATE_signed_char)
!169 = !DIDerivedType(tag: DW_TAG_member, name: "current_out_packet", scope: !118, file: !6, line: 228, baseType: !153, size: 64, offset: 896)
!170 = !DIDerivedType(tag: DW_TAG_member, name: "out_packet", scope: !118, file: !6, line: 229, baseType: !153, size: 64, offset: 960)
!171 = !DIDerivedType(tag: DW_TAG_member, name: "will", scope: !118, file: !6, line: 230, baseType: !172, size: 64, offset: 1024)
!172 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !173, size: 64)
!173 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_message_all", file: !6, line: 163, size: 704, elements: !174)
!174 = !{!175, !176, !177, !181, !182, !183, !185, !195}
!175 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !173, file: !6, line: 164, baseType: !172, size: 64)
!176 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !173, file: !6, line: 165, baseType: !172, size: 64, offset: 64)
!177 = !DIDerivedType(tag: DW_TAG_member, name: "properties", scope: !173, file: !6, line: 166, baseType: !178, size: 64, offset: 128)
!178 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !179, size: 64)
!179 = !DIDerivedType(tag: DW_TAG_typedef, name: "mosquitto_property", file: !77, line: 135, baseType: !180)
!180 = !DICompositeType(tag: DW_TAG_structure_type, name: "mqtt5__property", file: !77, line: 135, flags: DIFlagFwdDecl)
!181 = !DIDerivedType(tag: DW_TAG_member, name: "timestamp", scope: !173, file: !6, line: 167, baseType: !138, size: 64, offset: 192)
!182 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !173, file: !6, line: 169, baseType: !36, size: 32, offset: 256)
!183 = !DIDerivedType(tag: DW_TAG_member, name: "dup", scope: !173, file: !6, line: 170, baseType: !184, size: 8, offset: 288)
!184 = !DIBasicType(name: "_Bool", size: 8, encoding: DW_ATE_boolean)
!185 = !DIDerivedType(tag: DW_TAG_member, name: "msg", scope: !173, file: !6, line: 171, baseType: !186, size: 320, offset: 320)
!186 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_message", file: !77, line: 125, size: 320, elements: !187)
!187 = !{!188, !189, !190, !192, !193, !194}
!188 = !DIDerivedType(tag: DW_TAG_member, name: "mid", scope: !186, file: !77, line: 126, baseType: !116, size: 32)
!189 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !186, file: !77, line: 127, baseType: !124, size: 64, offset: 64)
!190 = !DIDerivedType(tag: DW_TAG_member, name: "payload", scope: !186, file: !77, line: 128, baseType: !191, size: 64, offset: 128)
!191 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!192 = !DIDerivedType(tag: DW_TAG_member, name: "payloadlen", scope: !186, file: !77, line: 129, baseType: !116, size: 32, offset: 192)
!193 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !186, file: !77, line: 130, baseType: !116, size: 32, offset: 224)
!194 = !DIDerivedType(tag: DW_TAG_member, name: "retain", scope: !186, file: !77, line: 131, baseType: !184, size: 8, offset: 256)
!195 = !DIDerivedType(tag: DW_TAG_member, name: "expiry_interval", scope: !173, file: !6, line: 172, baseType: !155, size: 32, offset: 640)
!196 = !DIDerivedType(tag: DW_TAG_member, name: "aliases", scope: !118, file: !6, line: 231, baseType: !197, size: 64, offset: 1088)
!197 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !198, size: 64)
!198 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__alias", file: !6, line: 139, size: 128, elements: !199)
!199 = !{!200, !201}
!200 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !198, file: !6, line: 140, baseType: !124, size: 64)
!201 = !DIDerivedType(tag: DW_TAG_member, name: "alias", scope: !198, file: !6, line: 141, baseType: !130, size: 16, offset: 64)
!202 = !DIDerivedType(tag: DW_TAG_member, name: "will_delay_entry", scope: !118, file: !6, line: 232, baseType: !203, size: 64, offset: 1152)
!203 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !204, size: 64)
!204 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "will_delay_list", file: !6, line: 182, size: 192, elements: !205)
!205 = !{!206, !207, !208}
!206 = !DIDerivedType(tag: DW_TAG_member, name: "context", scope: !204, file: !6, line: 183, baseType: !117, size: 64)
!207 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !204, file: !6, line: 184, baseType: !203, size: 64, offset: 64)
!208 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !204, file: !6, line: 185, baseType: !203, size: 64, offset: 128)
!209 = !DIDerivedType(tag: DW_TAG_member, name: "maximum_packet_size", scope: !118, file: !6, line: 233, baseType: !155, size: 32, offset: 1216)
!210 = !DIDerivedType(tag: DW_TAG_member, name: "alias_count", scope: !118, file: !6, line: 234, baseType: !116, size: 32, offset: 1248)
!211 = !DIDerivedType(tag: DW_TAG_member, name: "will_delay_interval", scope: !118, file: !6, line: 235, baseType: !155, size: 32, offset: 1280)
!212 = !DIDerivedType(tag: DW_TAG_member, name: "will_delay_time", scope: !118, file: !6, line: 236, baseType: !138, size: 64, offset: 1344)
!213 = !DIDerivedType(tag: DW_TAG_member, name: "ssl", scope: !118, file: !6, line: 238, baseType: !214, size: 64, offset: 1408)
!214 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !215, size: 64)
!215 = !DIDerivedType(tag: DW_TAG_typedef, name: "SSL", file: !216, line: 147, baseType: !217)
!216 = !DIFile(filename: "/usr/include/openssl/ossl_typ.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!217 = !DICompositeType(tag: DW_TAG_structure_type, name: "ssl_st", file: !216, line: 147, flags: DIFlagFwdDecl)
!218 = !DIDerivedType(tag: DW_TAG_member, name: "ssl_ctx", scope: !118, file: !6, line: 239, baseType: !219, size: 64, offset: 1472)
!219 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !220, size: 64)
!220 = !DIDerivedType(tag: DW_TAG_typedef, name: "SSL_CTX", file: !216, line: 148, baseType: !221)
!221 = !DICompositeType(tag: DW_TAG_structure_type, name: "ssl_ctx_st", file: !216, line: 148, flags: DIFlagFwdDecl)
!222 = !DIDerivedType(tag: DW_TAG_member, name: "tls_cafile", scope: !118, file: !6, line: 240, baseType: !124, size: 64, offset: 1536)
!223 = !DIDerivedType(tag: DW_TAG_member, name: "tls_capath", scope: !118, file: !6, line: 241, baseType: !124, size: 64, offset: 1600)
!224 = !DIDerivedType(tag: DW_TAG_member, name: "tls_certfile", scope: !118, file: !6, line: 242, baseType: !124, size: 64, offset: 1664)
!225 = !DIDerivedType(tag: DW_TAG_member, name: "tls_keyfile", scope: !118, file: !6, line: 243, baseType: !124, size: 64, offset: 1728)
!226 = !DIDerivedType(tag: DW_TAG_member, name: "tls_pw_callback", scope: !118, file: !6, line: 244, baseType: !227, size: 64, offset: 1792)
!227 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !228, size: 64)
!228 = !DISubroutineType(types: !229)
!229 = !{!116, !124, !116, !116, !191}
!230 = !DIDerivedType(tag: DW_TAG_member, name: "tls_version", scope: !118, file: !6, line: 245, baseType: !124, size: 64, offset: 1856)
!231 = !DIDerivedType(tag: DW_TAG_member, name: "tls_ciphers", scope: !118, file: !6, line: 246, baseType: !124, size: 64, offset: 1920)
!232 = !DIDerivedType(tag: DW_TAG_member, name: "tls_psk", scope: !118, file: !6, line: 247, baseType: !124, size: 64, offset: 1984)
!233 = !DIDerivedType(tag: DW_TAG_member, name: "tls_psk_identity", scope: !118, file: !6, line: 248, baseType: !124, size: 64, offset: 2048)
!234 = !DIDerivedType(tag: DW_TAG_member, name: "tls_cert_reqs", scope: !118, file: !6, line: 249, baseType: !116, size: 32, offset: 2112)
!235 = !DIDerivedType(tag: DW_TAG_member, name: "tls_insecure", scope: !118, file: !6, line: 250, baseType: !184, size: 8, offset: 2144)
!236 = !DIDerivedType(tag: DW_TAG_member, name: "ssl_ctx_defaults", scope: !118, file: !6, line: 251, baseType: !184, size: 8, offset: 2152)
!237 = !DIDerivedType(tag: DW_TAG_member, name: "tls_ocsp_required", scope: !118, file: !6, line: 252, baseType: !184, size: 8, offset: 2160)
!238 = !DIDerivedType(tag: DW_TAG_member, name: "tls_engine", scope: !118, file: !6, line: 253, baseType: !124, size: 64, offset: 2176)
!239 = !DIDerivedType(tag: DW_TAG_member, name: "tls_engine_kpass_sha1", scope: !118, file: !6, line: 254, baseType: !124, size: 64, offset: 2240)
!240 = !DIDerivedType(tag: DW_TAG_member, name: "tls_keyform", scope: !118, file: !6, line: 255, baseType: !50, size: 32, offset: 2304)
!241 = !DIDerivedType(tag: DW_TAG_member, name: "tls_alpn", scope: !118, file: !6, line: 256, baseType: !124, size: 64, offset: 2368)
!242 = !DIDerivedType(tag: DW_TAG_member, name: "want_write", scope: !118, file: !6, line: 258, baseType: !184, size: 8, offset: 2432)
!243 = !DIDerivedType(tag: DW_TAG_member, name: "want_connect", scope: !118, file: !6, line: 259, baseType: !184, size: 8, offset: 2440)
!244 = !DIDerivedType(tag: DW_TAG_member, name: "clean_start", scope: !118, file: !6, line: 270, baseType: !184, size: 8, offset: 2448)
!245 = !DIDerivedType(tag: DW_TAG_member, name: "session_expiry_interval", scope: !118, file: !6, line: 271, baseType: !155, size: 32, offset: 2464)
!246 = !DIDerivedType(tag: DW_TAG_member, name: "session_expiry_time", scope: !118, file: !6, line: 272, baseType: !138, size: 64, offset: 2496)
!247 = !DIDerivedType(tag: DW_TAG_member, name: "removed_from_by_id", scope: !118, file: !6, line: 274, baseType: !184, size: 8, offset: 2560)
!248 = !DIDerivedType(tag: DW_TAG_member, name: "is_dropping", scope: !118, file: !6, line: 275, baseType: !184, size: 8, offset: 2568)
!249 = !DIDerivedType(tag: DW_TAG_member, name: "is_bridge", scope: !118, file: !6, line: 276, baseType: !184, size: 8, offset: 2576)
!250 = !DIDerivedType(tag: DW_TAG_member, name: "bridge", scope: !118, file: !6, line: 277, baseType: !251, size: 64, offset: 2624)
!251 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !252, size: 64)
!252 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__bridge", file: !55, line: 484, size: 1920, elements: !253)
!253 = !{!254, !255, !261, !262, !263, !264, !265, !266, !267, !268, !269, !270, !281, !282, !283, !284, !285, !286, !287, !288, !289, !290, !291, !292, !293, !294, !295, !296, !297, !298, !299, !300, !301, !302, !303, !304, !305, !306, !307, !308, !309, !310, !311, !312}
!254 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !252, file: !55, line: 485, baseType: !124, size: 64)
!255 = !DIDerivedType(tag: DW_TAG_member, name: "addresses", scope: !252, file: !55, line: 486, baseType: !256, size: 64, offset: 64)
!256 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !257, size: 64)
!257 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bridge_address", file: !55, line: 479, size: 128, elements: !258)
!258 = !{!259, !260}
!259 = !DIDerivedType(tag: DW_TAG_member, name: "address", scope: !257, file: !55, line: 480, baseType: !124, size: 64)
!260 = !DIDerivedType(tag: DW_TAG_member, name: "port", scope: !257, file: !55, line: 481, baseType: !116, size: 32, offset: 64)
!261 = !DIDerivedType(tag: DW_TAG_member, name: "cur_address", scope: !252, file: !55, line: 487, baseType: !116, size: 32, offset: 128)
!262 = !DIDerivedType(tag: DW_TAG_member, name: "address_count", scope: !252, file: !55, line: 488, baseType: !116, size: 32, offset: 160)
!263 = !DIDerivedType(tag: DW_TAG_member, name: "primary_retry", scope: !252, file: !55, line: 489, baseType: !138, size: 64, offset: 192)
!264 = !DIDerivedType(tag: DW_TAG_member, name: "primary_retry_sock", scope: !252, file: !55, line: 490, baseType: !121, size: 32, offset: 256)
!265 = !DIDerivedType(tag: DW_TAG_member, name: "round_robin", scope: !252, file: !55, line: 491, baseType: !184, size: 8, offset: 288)
!266 = !DIDerivedType(tag: DW_TAG_member, name: "try_private", scope: !252, file: !55, line: 492, baseType: !184, size: 8, offset: 296)
!267 = !DIDerivedType(tag: DW_TAG_member, name: "try_private_accepted", scope: !252, file: !55, line: 493, baseType: !184, size: 8, offset: 304)
!268 = !DIDerivedType(tag: DW_TAG_member, name: "clean_start", scope: !252, file: !55, line: 494, baseType: !184, size: 8, offset: 312)
!269 = !DIDerivedType(tag: DW_TAG_member, name: "keepalive", scope: !252, file: !55, line: 495, baseType: !116, size: 32, offset: 320)
!270 = !DIDerivedType(tag: DW_TAG_member, name: "topics", scope: !252, file: !55, line: 496, baseType: !271, size: 64, offset: 384)
!271 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !272, size: 64)
!272 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__bridge_topic", file: !55, line: 469, size: 384, elements: !273)
!273 = !{!274, !275, !276, !277, !278, !279, !280}
!274 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !272, file: !55, line: 470, baseType: !124, size: 64)
!275 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !272, file: !55, line: 471, baseType: !116, size: 32, offset: 64)
!276 = !DIDerivedType(tag: DW_TAG_member, name: "direction", scope: !272, file: !55, line: 472, baseType: !54, size: 32, offset: 96)
!277 = !DIDerivedType(tag: DW_TAG_member, name: "local_prefix", scope: !272, file: !55, line: 473, baseType: !124, size: 64, offset: 128)
!278 = !DIDerivedType(tag: DW_TAG_member, name: "remote_prefix", scope: !272, file: !55, line: 474, baseType: !124, size: 64, offset: 192)
!279 = !DIDerivedType(tag: DW_TAG_member, name: "local_topic", scope: !272, file: !55, line: 475, baseType: !124, size: 64, offset: 256)
!280 = !DIDerivedType(tag: DW_TAG_member, name: "remote_topic", scope: !272, file: !55, line: 476, baseType: !124, size: 64, offset: 320)
!281 = !DIDerivedType(tag: DW_TAG_member, name: "topic_count", scope: !252, file: !55, line: 497, baseType: !116, size: 32, offset: 448)
!282 = !DIDerivedType(tag: DW_TAG_member, name: "topic_remapping", scope: !252, file: !55, line: 498, baseType: !184, size: 8, offset: 480)
!283 = !DIDerivedType(tag: DW_TAG_member, name: "protocol_version", scope: !252, file: !55, line: 499, baseType: !5, size: 32, offset: 512)
!284 = !DIDerivedType(tag: DW_TAG_member, name: "restart_t", scope: !252, file: !55, line: 500, baseType: !138, size: 64, offset: 576)
!285 = !DIDerivedType(tag: DW_TAG_member, name: "remote_clientid", scope: !252, file: !55, line: 501, baseType: !124, size: 64, offset: 640)
!286 = !DIDerivedType(tag: DW_TAG_member, name: "remote_username", scope: !252, file: !55, line: 502, baseType: !124, size: 64, offset: 704)
!287 = !DIDerivedType(tag: DW_TAG_member, name: "remote_password", scope: !252, file: !55, line: 503, baseType: !124, size: 64, offset: 768)
!288 = !DIDerivedType(tag: DW_TAG_member, name: "local_clientid", scope: !252, file: !55, line: 504, baseType: !124, size: 64, offset: 832)
!289 = !DIDerivedType(tag: DW_TAG_member, name: "local_username", scope: !252, file: !55, line: 505, baseType: !124, size: 64, offset: 896)
!290 = !DIDerivedType(tag: DW_TAG_member, name: "local_password", scope: !252, file: !55, line: 506, baseType: !124, size: 64, offset: 960)
!291 = !DIDerivedType(tag: DW_TAG_member, name: "notification_topic", scope: !252, file: !55, line: 507, baseType: !124, size: 64, offset: 1024)
!292 = !DIDerivedType(tag: DW_TAG_member, name: "notifications", scope: !252, file: !55, line: 508, baseType: !184, size: 8, offset: 1088)
!293 = !DIDerivedType(tag: DW_TAG_member, name: "notifications_local_only", scope: !252, file: !55, line: 509, baseType: !184, size: 8, offset: 1096)
!294 = !DIDerivedType(tag: DW_TAG_member, name: "start_type", scope: !252, file: !55, line: 510, baseType: !60, size: 32, offset: 1120)
!295 = !DIDerivedType(tag: DW_TAG_member, name: "idle_timeout", scope: !252, file: !55, line: 511, baseType: !116, size: 32, offset: 1152)
!296 = !DIDerivedType(tag: DW_TAG_member, name: "restart_timeout", scope: !252, file: !55, line: 512, baseType: !116, size: 32, offset: 1184)
!297 = !DIDerivedType(tag: DW_TAG_member, name: "backoff_base", scope: !252, file: !55, line: 513, baseType: !116, size: 32, offset: 1216)
!298 = !DIDerivedType(tag: DW_TAG_member, name: "backoff_cap", scope: !252, file: !55, line: 514, baseType: !116, size: 32, offset: 1248)
!299 = !DIDerivedType(tag: DW_TAG_member, name: "threshold", scope: !252, file: !55, line: 515, baseType: !116, size: 32, offset: 1280)
!300 = !DIDerivedType(tag: DW_TAG_member, name: "lazy_reconnect", scope: !252, file: !55, line: 516, baseType: !184, size: 8, offset: 1312)
!301 = !DIDerivedType(tag: DW_TAG_member, name: "attempt_unsubscribe", scope: !252, file: !55, line: 517, baseType: !184, size: 8, offset: 1320)
!302 = !DIDerivedType(tag: DW_TAG_member, name: "initial_notification_done", scope: !252, file: !55, line: 518, baseType: !184, size: 8, offset: 1328)
!303 = !DIDerivedType(tag: DW_TAG_member, name: "tls_insecure", scope: !252, file: !55, line: 520, baseType: !184, size: 8, offset: 1336)
!304 = !DIDerivedType(tag: DW_TAG_member, name: "tls_ocsp_required", scope: !252, file: !55, line: 521, baseType: !184, size: 8, offset: 1344)
!305 = !DIDerivedType(tag: DW_TAG_member, name: "tls_cafile", scope: !252, file: !55, line: 522, baseType: !124, size: 64, offset: 1408)
!306 = !DIDerivedType(tag: DW_TAG_member, name: "tls_capath", scope: !252, file: !55, line: 523, baseType: !124, size: 64, offset: 1472)
!307 = !DIDerivedType(tag: DW_TAG_member, name: "tls_certfile", scope: !252, file: !55, line: 524, baseType: !124, size: 64, offset: 1536)
!308 = !DIDerivedType(tag: DW_TAG_member, name: "tls_keyfile", scope: !252, file: !55, line: 525, baseType: !124, size: 64, offset: 1600)
!309 = !DIDerivedType(tag: DW_TAG_member, name: "tls_version", scope: !252, file: !55, line: 526, baseType: !124, size: 64, offset: 1664)
!310 = !DIDerivedType(tag: DW_TAG_member, name: "tls_alpn", scope: !252, file: !55, line: 527, baseType: !124, size: 64, offset: 1728)
!311 = !DIDerivedType(tag: DW_TAG_member, name: "tls_psk_identity", scope: !252, file: !55, line: 529, baseType: !124, size: 64, offset: 1792)
!312 = !DIDerivedType(tag: DW_TAG_member, name: "tls_psk", scope: !252, file: !55, line: 530, baseType: !124, size: 64, offset: 1856)
!313 = !DIDerivedType(tag: DW_TAG_member, name: "msgs_in", scope: !118, file: !6, line: 278, baseType: !314, size: 384, offset: 2688)
!314 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_msg_data", file: !6, line: 188, size: 384, elements: !315)
!315 = !{!316, !617, !618, !619, !620, !621, !622, !623}
!316 = !DIDerivedType(tag: DW_TAG_member, name: "inflight", scope: !314, file: !6, line: 190, baseType: !317, size: 64)
!317 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !318, size: 64)
!318 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_client_msg", file: !55, line: 382, size: 448, elements: !319)
!319 = !{!320, !321, !322, !609, !610, !611, !612, !613, !614, !615, !616}
!320 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !318, file: !55, line: 383, baseType: !317, size: 64)
!321 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !318, file: !55, line: 384, baseType: !317, size: 64, offset: 64)
!322 = !DIDerivedType(tag: DW_TAG_member, name: "store", scope: !318, file: !55, line: 385, baseType: !323, size: 64, offset: 128)
!323 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !324, size: 64)
!324 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_msg_store", file: !55, line: 360, size: 896, elements: !325)
!325 = !{!326, !327, !328, !333, !334, !335, !587, !589, !590, !591, !592, !593, !602, !603, !604, !605, !606, !607, !608}
!326 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !324, file: !55, line: 361, baseType: !323, size: 64)
!327 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !324, file: !55, line: 362, baseType: !323, size: 64, offset: 64)
!328 = !DIDerivedType(tag: DW_TAG_member, name: "db_id", scope: !324, file: !55, line: 363, baseType: !329, size: 64, offset: 128)
!329 = !DIDerivedType(tag: DW_TAG_typedef, name: "dbid_t", file: !55, line: 135, baseType: !330)
!330 = !DIDerivedType(tag: DW_TAG_typedef, name: "uint64_t", file: !131, line: 27, baseType: !331)
!331 = !DIDerivedType(tag: DW_TAG_typedef, name: "__uint64_t", file: !133, line: 44, baseType: !332)
!332 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!333 = !DIDerivedType(tag: DW_TAG_member, name: "source_id", scope: !324, file: !55, line: 364, baseType: !124, size: 64, offset: 192)
!334 = !DIDerivedType(tag: DW_TAG_member, name: "source_username", scope: !324, file: !55, line: 365, baseType: !124, size: 64, offset: 256)
!335 = !DIDerivedType(tag: DW_TAG_member, name: "source_listener", scope: !324, file: !55, line: 366, baseType: !336, size: 64, offset: 320)
!336 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !337, size: 64)
!337 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__listener", file: !55, line: 230, size: 2112, elements: !338)
!338 = !{!339, !340, !341, !342, !343, !344, !345, !347, !348, !349, !350, !351, !352, !353, !354, !355, !356, !357, !358, !359, !360, !361, !362, !363, !364, !365, !366, !367, !368, !369, !370, !539, !586}
!339 = !DIDerivedType(tag: DW_TAG_member, name: "fd", scope: !337, file: !55, line: 231, baseType: !116, size: 32)
!340 = !DIDerivedType(tag: DW_TAG_member, name: "port", scope: !337, file: !55, line: 232, baseType: !130, size: 16, offset: 32)
!341 = !DIDerivedType(tag: DW_TAG_member, name: "host", scope: !337, file: !55, line: 233, baseType: !124, size: 64, offset: 64)
!342 = !DIDerivedType(tag: DW_TAG_member, name: "bind_interface", scope: !337, file: !55, line: 234, baseType: !124, size: 64, offset: 128)
!343 = !DIDerivedType(tag: DW_TAG_member, name: "max_connections", scope: !337, file: !55, line: 235, baseType: !116, size: 32, offset: 192)
!344 = !DIDerivedType(tag: DW_TAG_member, name: "mount_point", scope: !337, file: !55, line: 236, baseType: !124, size: 64, offset: 256)
!345 = !DIDerivedType(tag: DW_TAG_member, name: "socks", scope: !337, file: !55, line: 237, baseType: !346, size: 64, offset: 320)
!346 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !121, size: 64)
!347 = !DIDerivedType(tag: DW_TAG_member, name: "sock_count", scope: !337, file: !55, line: 238, baseType: !116, size: 32, offset: 384)
!348 = !DIDerivedType(tag: DW_TAG_member, name: "client_count", scope: !337, file: !55, line: 239, baseType: !116, size: 32, offset: 416)
!349 = !DIDerivedType(tag: DW_TAG_member, name: "protocol", scope: !337, file: !55, line: 240, baseType: !66, size: 32, offset: 448)
!350 = !DIDerivedType(tag: DW_TAG_member, name: "socket_domain", scope: !337, file: !55, line: 241, baseType: !116, size: 32, offset: 480)
!351 = !DIDerivedType(tag: DW_TAG_member, name: "use_username_as_clientid", scope: !337, file: !55, line: 242, baseType: !184, size: 8, offset: 512)
!352 = !DIDerivedType(tag: DW_TAG_member, name: "maximum_qos", scope: !337, file: !55, line: 243, baseType: !149, size: 8, offset: 520)
!353 = !DIDerivedType(tag: DW_TAG_member, name: "max_topic_alias", scope: !337, file: !55, line: 244, baseType: !130, size: 16, offset: 528)
!354 = !DIDerivedType(tag: DW_TAG_member, name: "cafile", scope: !337, file: !55, line: 246, baseType: !124, size: 64, offset: 576)
!355 = !DIDerivedType(tag: DW_TAG_member, name: "capath", scope: !337, file: !55, line: 247, baseType: !124, size: 64, offset: 640)
!356 = !DIDerivedType(tag: DW_TAG_member, name: "certfile", scope: !337, file: !55, line: 248, baseType: !124, size: 64, offset: 704)
!357 = !DIDerivedType(tag: DW_TAG_member, name: "keyfile", scope: !337, file: !55, line: 249, baseType: !124, size: 64, offset: 768)
!358 = !DIDerivedType(tag: DW_TAG_member, name: "tls_engine", scope: !337, file: !55, line: 250, baseType: !124, size: 64, offset: 832)
!359 = !DIDerivedType(tag: DW_TAG_member, name: "tls_engine_kpass_sha1", scope: !337, file: !55, line: 251, baseType: !124, size: 64, offset: 896)
!360 = !DIDerivedType(tag: DW_TAG_member, name: "ciphers", scope: !337, file: !55, line: 252, baseType: !124, size: 64, offset: 960)
!361 = !DIDerivedType(tag: DW_TAG_member, name: "psk_hint", scope: !337, file: !55, line: 253, baseType: !124, size: 64, offset: 1024)
!362 = !DIDerivedType(tag: DW_TAG_member, name: "ssl_ctx", scope: !337, file: !55, line: 254, baseType: !219, size: 64, offset: 1088)
!363 = !DIDerivedType(tag: DW_TAG_member, name: "crlfile", scope: !337, file: !55, line: 255, baseType: !124, size: 64, offset: 1152)
!364 = !DIDerivedType(tag: DW_TAG_member, name: "tls_version", scope: !337, file: !55, line: 256, baseType: !124, size: 64, offset: 1216)
!365 = !DIDerivedType(tag: DW_TAG_member, name: "dhparamfile", scope: !337, file: !55, line: 257, baseType: !124, size: 64, offset: 1280)
!366 = !DIDerivedType(tag: DW_TAG_member, name: "use_identity_as_username", scope: !337, file: !55, line: 258, baseType: !184, size: 8, offset: 1344)
!367 = !DIDerivedType(tag: DW_TAG_member, name: "use_subject_as_username", scope: !337, file: !55, line: 259, baseType: !184, size: 8, offset: 1352)
!368 = !DIDerivedType(tag: DW_TAG_member, name: "require_certificate", scope: !337, file: !55, line: 260, baseType: !184, size: 8, offset: 1360)
!369 = !DIDerivedType(tag: DW_TAG_member, name: "tls_keyform", scope: !337, file: !55, line: 261, baseType: !50, size: 32, offset: 1376)
!370 = !DIDerivedType(tag: DW_TAG_member, name: "security_options", scope: !337, file: !55, line: 268, baseType: !371, size: 576, offset: 1408)
!371 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__security_options", file: !55, line: 212, size: 576, elements: !372)
!372 = !{!373, !388, !389, !390, !391, !392, !534, !535, !536, !537, !538}
!373 = !DIDerivedType(tag: DW_TAG_member, name: "acl_list", scope: !371, file: !55, line: 217, baseType: !374, size: 64)
!374 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !375, size: 64)
!375 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__acl_user", file: !55, line: 415, size: 192, elements: !376)
!376 = !{!377, !378, !379}
!377 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !375, file: !55, line: 416, baseType: !374, size: 64)
!378 = !DIDerivedType(tag: DW_TAG_member, name: "username", scope: !375, file: !55, line: 417, baseType: !124, size: 64, offset: 64)
!379 = !DIDerivedType(tag: DW_TAG_member, name: "acl", scope: !375, file: !55, line: 418, baseType: !380, size: 64, offset: 128)
!380 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !381, size: 64)
!381 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__acl", file: !55, line: 407, size: 256, elements: !382)
!382 = !{!383, !384, !385, !386, !387}
!383 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !381, file: !55, line: 408, baseType: !380, size: 64)
!384 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !381, file: !55, line: 409, baseType: !124, size: 64, offset: 64)
!385 = !DIDerivedType(tag: DW_TAG_member, name: "access", scope: !381, file: !55, line: 410, baseType: !116, size: 32, offset: 128)
!386 = !DIDerivedType(tag: DW_TAG_member, name: "ucount", scope: !381, file: !55, line: 411, baseType: !116, size: 32, offset: 160)
!387 = !DIDerivedType(tag: DW_TAG_member, name: "ccount", scope: !381, file: !55, line: 412, baseType: !116, size: 32, offset: 192)
!388 = !DIDerivedType(tag: DW_TAG_member, name: "acl_patterns", scope: !371, file: !55, line: 218, baseType: !380, size: 64, offset: 64)
!389 = !DIDerivedType(tag: DW_TAG_member, name: "password_file", scope: !371, file: !55, line: 219, baseType: !124, size: 64, offset: 128)
!390 = !DIDerivedType(tag: DW_TAG_member, name: "psk_file", scope: !371, file: !55, line: 220, baseType: !124, size: 64, offset: 192)
!391 = !DIDerivedType(tag: DW_TAG_member, name: "acl_file", scope: !371, file: !55, line: 221, baseType: !124, size: 64, offset: 256)
!392 = !DIDerivedType(tag: DW_TAG_member, name: "auth_plugin_configs", scope: !371, file: !55, line: 222, baseType: !393, size: 64, offset: 320)
!393 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !394, size: 64)
!394 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__auth_plugin_config", file: !55, line: 202, size: 1920, elements: !395)
!395 = !{!396, !397, !404, !405, !406}
!396 = !DIDerivedType(tag: DW_TAG_member, name: "path", scope: !394, file: !55, line: 204, baseType: !124, size: 64)
!397 = !DIDerivedType(tag: DW_TAG_member, name: "options", scope: !394, file: !55, line: 205, baseType: !398, size: 64, offset: 64)
!398 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !399, size: 64)
!399 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_opt", file: !400, line: 35, size: 128, elements: !401)
!400 = !DIFile(filename: "./mosquitto_plugin.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!401 = !{!402, !403}
!402 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !399, file: !400, line: 36, baseType: !124, size: 64)
!403 = !DIDerivedType(tag: DW_TAG_member, name: "value", scope: !399, file: !400, line: 37, baseType: !124, size: 64, offset: 64)
!404 = !DIDerivedType(tag: DW_TAG_member, name: "option_count", scope: !394, file: !55, line: 206, baseType: !116, size: 32, offset: 128)
!405 = !DIDerivedType(tag: DW_TAG_member, name: "deny_special_chars", scope: !394, file: !55, line: 207, baseType: !184, size: 8, offset: 160)
!406 = !DIDerivedType(tag: DW_TAG_member, name: "plugin", scope: !394, file: !55, line: 209, baseType: !407, size: 1728, offset: 192)
!407 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__auth_plugin", file: !55, line: 169, size: 1728, elements: !408)
!408 = !{!409, !410, !411, !415, !421, !426, !431, !433, !450, !455, !460, !466, !471, !473, !475, !477, !479, !486, !491, !496, !506, !511, !516, !518, !523, !528, !533}
!409 = !DIDerivedType(tag: DW_TAG_member, name: "lib", scope: !407, file: !55, line: 170, baseType: !191, size: 64)
!410 = !DIDerivedType(tag: DW_TAG_member, name: "user_data", scope: !407, file: !55, line: 171, baseType: !191, size: 64, offset: 64)
!411 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_version", scope: !407, file: !55, line: 172, baseType: !412, size: 64, offset: 128)
!412 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !413, size: 64)
!413 = !DISubroutineType(types: !414)
!414 = !{!116}
!415 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_init_v4", scope: !407, file: !55, line: 174, baseType: !416, size: 64, offset: 192)
!416 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_init_v4", file: !55, line: 137, baseType: !417)
!417 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !418, size: 64)
!418 = !DISubroutineType(types: !419)
!419 = !{!116, !420, !398, !116}
!420 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !191, size: 64)
!421 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_cleanup_v4", scope: !407, file: !55, line: 175, baseType: !422, size: 64, offset: 256)
!422 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_cleanup_v4", file: !55, line: 138, baseType: !423)
!423 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !424, size: 64)
!424 = !DISubroutineType(types: !425)
!425 = !{!116, !191, !398, !116}
!426 = !DIDerivedType(tag: DW_TAG_member, name: "security_init_v4", scope: !407, file: !55, line: 176, baseType: !427, size: 64, offset: 320)
!427 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_security_init_v4", file: !55, line: 139, baseType: !428)
!428 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !429, size: 64)
!429 = !DISubroutineType(types: !430)
!430 = !{!116, !191, !398, !116, !184}
!431 = !DIDerivedType(tag: DW_TAG_member, name: "security_cleanup_v4", scope: !407, file: !55, line: 177, baseType: !432, size: 64, offset: 384)
!432 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_security_cleanup_v4", file: !55, line: 140, baseType: !428)
!433 = !DIDerivedType(tag: DW_TAG_member, name: "acl_check_v4", scope: !407, file: !55, line: 178, baseType: !434, size: 64, offset: 448)
!434 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_acl_check_v4", file: !55, line: 141, baseType: !435)
!435 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !436, size: 64)
!436 = !DISubroutineType(types: !437)
!437 = !{!116, !191, !116, !117, !438}
!438 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !439, size: 64)
!439 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_acl_msg", file: !400, line: 45, size: 256, elements: !440)
!440 = !{!441, !444, !447, !448, !449}
!441 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !439, file: !400, line: 46, baseType: !442, size: 64)
!442 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !443, size: 64)
!443 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !125)
!444 = !DIDerivedType(tag: DW_TAG_member, name: "payload", scope: !439, file: !400, line: 47, baseType: !445, size: 64, offset: 64)
!445 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !446, size: 64)
!446 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!447 = !DIDerivedType(tag: DW_TAG_member, name: "payloadlen", scope: !439, file: !400, line: 48, baseType: !141, size: 64, offset: 128)
!448 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !439, file: !400, line: 49, baseType: !116, size: 32, offset: 192)
!449 = !DIDerivedType(tag: DW_TAG_member, name: "retain", scope: !439, file: !400, line: 50, baseType: !184, size: 8, offset: 224)
!450 = !DIDerivedType(tag: DW_TAG_member, name: "unpwd_check_v4", scope: !407, file: !55, line: 179, baseType: !451, size: 64, offset: 512)
!451 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_unpwd_check_v4", file: !55, line: 142, baseType: !452)
!452 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !453, size: 64)
!453 = !DISubroutineType(types: !454)
!454 = !{!116, !191, !117, !442, !442}
!455 = !DIDerivedType(tag: DW_TAG_member, name: "psk_key_get_v4", scope: !407, file: !55, line: 180, baseType: !456, size: 64, offset: 576)
!456 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_psk_key_get_v4", file: !55, line: 143, baseType: !457)
!457 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !458, size: 64)
!458 = !DISubroutineType(types: !459)
!459 = !{!116, !191, !117, !442, !442, !124, !116}
!460 = !DIDerivedType(tag: DW_TAG_member, name: "auth_start_v4", scope: !407, file: !55, line: 181, baseType: !461, size: 64, offset: 640)
!461 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_auth_start_v4", file: !55, line: 144, baseType: !462)
!462 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !463, size: 64)
!463 = !DISubroutineType(types: !464)
!464 = !{!116, !191, !117, !442, !184, !445, !130, !420, !465}
!465 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !130, size: 64)
!466 = !DIDerivedType(tag: DW_TAG_member, name: "auth_continue_v4", scope: !407, file: !55, line: 182, baseType: !467, size: 64, offset: 704)
!467 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_auth_continue_v4", file: !55, line: 145, baseType: !468)
!468 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !469, size: 64)
!469 = !DISubroutineType(types: !470)
!470 = !{!116, !191, !117, !442, !445, !130, !420, !465}
!471 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_init_v3", scope: !407, file: !55, line: 184, baseType: !472, size: 64, offset: 768)
!472 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_init_v3", file: !55, line: 147, baseType: !417)
!473 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_cleanup_v3", scope: !407, file: !55, line: 185, baseType: !474, size: 64, offset: 832)
!474 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_cleanup_v3", file: !55, line: 148, baseType: !423)
!475 = !DIDerivedType(tag: DW_TAG_member, name: "security_init_v3", scope: !407, file: !55, line: 186, baseType: !476, size: 64, offset: 896)
!476 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_security_init_v3", file: !55, line: 149, baseType: !428)
!477 = !DIDerivedType(tag: DW_TAG_member, name: "security_cleanup_v3", scope: !407, file: !55, line: 187, baseType: !478, size: 64, offset: 960)
!478 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_security_cleanup_v3", file: !55, line: 150, baseType: !428)
!479 = !DIDerivedType(tag: DW_TAG_member, name: "acl_check_v3", scope: !407, file: !55, line: 188, baseType: !480, size: 64, offset: 1024)
!480 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_acl_check_v3", file: !55, line: 151, baseType: !481)
!481 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !482, size: 64)
!482 = !DISubroutineType(types: !483)
!483 = !{!116, !191, !116, !484, !438}
!484 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !485, size: 64)
!485 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !118)
!486 = !DIDerivedType(tag: DW_TAG_member, name: "unpwd_check_v3", scope: !407, file: !55, line: 189, baseType: !487, size: 64, offset: 1088)
!487 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_unpwd_check_v3", file: !55, line: 152, baseType: !488)
!488 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !489, size: 64)
!489 = !DISubroutineType(types: !490)
!490 = !{!116, !191, !484, !442, !442}
!491 = !DIDerivedType(tag: DW_TAG_member, name: "psk_key_get_v3", scope: !407, file: !55, line: 190, baseType: !492, size: 64, offset: 1152)
!492 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_psk_key_get_v3", file: !55, line: 153, baseType: !493)
!493 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !494, size: 64)
!494 = !DISubroutineType(types: !495)
!495 = !{!116, !191, !484, !442, !442, !124, !116}
!496 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_init_v2", scope: !407, file: !55, line: 192, baseType: !497, size: 64, offset: 1216)
!497 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_init_v2", file: !55, line: 155, baseType: !498)
!498 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !499, size: 64)
!499 = !DISubroutineType(types: !500)
!500 = !{!116, !420, !501, !116}
!501 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !502, size: 64)
!502 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_auth_opt", file: !400, line: 40, size: 128, elements: !503)
!503 = !{!504, !505}
!504 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !502, file: !400, line: 41, baseType: !124, size: 64)
!505 = !DIDerivedType(tag: DW_TAG_member, name: "value", scope: !502, file: !400, line: 42, baseType: !124, size: 64, offset: 64)
!506 = !DIDerivedType(tag: DW_TAG_member, name: "plugin_cleanup_v2", scope: !407, file: !55, line: 193, baseType: !507, size: 64, offset: 1280)
!507 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_cleanup_v2", file: !55, line: 156, baseType: !508)
!508 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !509, size: 64)
!509 = !DISubroutineType(types: !510)
!510 = !{!116, !191, !501, !116}
!511 = !DIDerivedType(tag: DW_TAG_member, name: "security_init_v2", scope: !407, file: !55, line: 194, baseType: !512, size: 64, offset: 1344)
!512 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_security_init_v2", file: !55, line: 157, baseType: !513)
!513 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !514, size: 64)
!514 = !DISubroutineType(types: !515)
!515 = !{!116, !191, !501, !116, !184}
!516 = !DIDerivedType(tag: DW_TAG_member, name: "security_cleanup_v2", scope: !407, file: !55, line: 195, baseType: !517, size: 64, offset: 1408)
!517 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_security_cleanup_v2", file: !55, line: 158, baseType: !513)
!518 = !DIDerivedType(tag: DW_TAG_member, name: "acl_check_v2", scope: !407, file: !55, line: 196, baseType: !519, size: 64, offset: 1472)
!519 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_acl_check_v2", file: !55, line: 159, baseType: !520)
!520 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !521, size: 64)
!521 = !DISubroutineType(types: !522)
!522 = !{!116, !191, !442, !442, !442, !116}
!523 = !DIDerivedType(tag: DW_TAG_member, name: "unpwd_check_v2", scope: !407, file: !55, line: 197, baseType: !524, size: 64, offset: 1536)
!524 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_unpwd_check_v2", file: !55, line: 160, baseType: !525)
!525 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !526, size: 64)
!526 = !DISubroutineType(types: !527)
!527 = !{!116, !191, !442, !442}
!528 = !DIDerivedType(tag: DW_TAG_member, name: "psk_key_get_v2", scope: !407, file: !55, line: 198, baseType: !529, size: 64, offset: 1600)
!529 = !DIDerivedType(tag: DW_TAG_typedef, name: "FUNC_auth_plugin_psk_key_get_v2", file: !55, line: 161, baseType: !530)
!530 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !531, size: 64)
!531 = !DISubroutineType(types: !532)
!532 = !{!116, !191, !442, !442, !124, !116}
!533 = !DIDerivedType(tag: DW_TAG_member, name: "version", scope: !407, file: !55, line: 199, baseType: !116, size: 32, offset: 1664)
!534 = !DIDerivedType(tag: DW_TAG_member, name: "auth_plugin_config_count", scope: !371, file: !55, line: 223, baseType: !116, size: 32, offset: 384)
!535 = !DIDerivedType(tag: DW_TAG_member, name: "allow_anonymous", scope: !371, file: !55, line: 224, baseType: !165, size: 8, offset: 416)
!536 = !DIDerivedType(tag: DW_TAG_member, name: "allow_zero_length_clientid", scope: !371, file: !55, line: 225, baseType: !184, size: 8, offset: 424)
!537 = !DIDerivedType(tag: DW_TAG_member, name: "auto_id_prefix", scope: !371, file: !55, line: 226, baseType: !124, size: 64, offset: 448)
!538 = !DIDerivedType(tag: DW_TAG_member, name: "auto_id_prefix_len", scope: !371, file: !55, line: 227, baseType: !116, size: 32, offset: 512)
!539 = !DIDerivedType(tag: DW_TAG_member, name: "unpwd", scope: !337, file: !55, line: 269, baseType: !540, size: 64, offset: 1984)
!540 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !541, size: 64)
!541 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__unpwd", file: !55, line: 396, size: 704, elements: !542)
!542 = !{!543, !544, !545, !546, !547, !549}
!543 = !DIDerivedType(tag: DW_TAG_member, name: "username", scope: !541, file: !55, line: 397, baseType: !124, size: 64)
!544 = !DIDerivedType(tag: DW_TAG_member, name: "password", scope: !541, file: !55, line: 398, baseType: !124, size: 64, offset: 64)
!545 = !DIDerivedType(tag: DW_TAG_member, name: "password_len", scope: !541, file: !55, line: 400, baseType: !157, size: 32, offset: 128)
!546 = !DIDerivedType(tag: DW_TAG_member, name: "salt_len", scope: !541, file: !55, line: 401, baseType: !157, size: 32, offset: 160)
!547 = !DIDerivedType(tag: DW_TAG_member, name: "salt", scope: !541, file: !55, line: 402, baseType: !548, size: 64, offset: 192)
!548 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !151, size: 64)
!549 = !DIDerivedType(tag: DW_TAG_member, name: "hh", scope: !541, file: !55, line: 404, baseType: !550, size: 448, offset: 256)
!550 = !DIDerivedType(tag: DW_TAG_typedef, name: "UT_hash_handle", file: !551, line: 946, baseType: !552)
!551 = !DIFile(filename: "../src/deps/uthash.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!552 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "UT_hash_handle", file: !551, line: 937, size: 448, elements: !553)
!553 = !{!554, !579, !580, !581, !582, !583, !584, !585}
!554 = !DIDerivedType(tag: DW_TAG_member, name: "tbl", scope: !552, file: !551, line: 938, baseType: !555, size: 64)
!555 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !556, size: 64)
!556 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "UT_hash_table", file: !551, line: 904, size: 512, elements: !557)
!557 = !{!558, !567, !568, !569, !570, !571, !574, !575, !576, !577, !578}
!558 = !DIDerivedType(tag: DW_TAG_member, name: "buckets", scope: !556, file: !551, line: 905, baseType: !559, size: 64)
!559 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !560, size: 64)
!560 = !DIDerivedType(tag: DW_TAG_typedef, name: "UT_hash_bucket", file: !551, line: 898, baseType: !561)
!561 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "UT_hash_bucket", file: !551, line: 880, size: 128, elements: !562)
!562 = !{!563, !565, !566}
!563 = !DIDerivedType(tag: DW_TAG_member, name: "hh_head", scope: !561, file: !551, line: 881, baseType: !564, size: 64)
!564 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !552, size: 64)
!565 = !DIDerivedType(tag: DW_TAG_member, name: "count", scope: !561, file: !551, line: 882, baseType: !157, size: 32, offset: 64)
!566 = !DIDerivedType(tag: DW_TAG_member, name: "expand_mult", scope: !561, file: !551, line: 896, baseType: !157, size: 32, offset: 96)
!567 = !DIDerivedType(tag: DW_TAG_member, name: "num_buckets", scope: !556, file: !551, line: 906, baseType: !157, size: 32, offset: 64)
!568 = !DIDerivedType(tag: DW_TAG_member, name: "log2_num_buckets", scope: !556, file: !551, line: 906, baseType: !157, size: 32, offset: 96)
!569 = !DIDerivedType(tag: DW_TAG_member, name: "num_items", scope: !556, file: !551, line: 907, baseType: !157, size: 32, offset: 128)
!570 = !DIDerivedType(tag: DW_TAG_member, name: "tail", scope: !556, file: !551, line: 908, baseType: !564, size: 64, offset: 192)
!571 = !DIDerivedType(tag: DW_TAG_member, name: "hho", scope: !556, file: !551, line: 909, baseType: !572, size: 64, offset: 256)
!572 = !DIDerivedType(tag: DW_TAG_typedef, name: "ptrdiff_t", file: !573, line: 51, baseType: !141)
!573 = !DIFile(filename: "/usr/lib/llvm-6.0/lib/clang/6.0.0/include/stddef.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!574 = !DIDerivedType(tag: DW_TAG_member, name: "ideal_chain_maxlen", scope: !556, file: !551, line: 913, baseType: !157, size: 32, offset: 320)
!575 = !DIDerivedType(tag: DW_TAG_member, name: "nonideal_items", scope: !556, file: !551, line: 918, baseType: !157, size: 32, offset: 352)
!576 = !DIDerivedType(tag: DW_TAG_member, name: "ineff_expands", scope: !556, file: !551, line: 926, baseType: !157, size: 32, offset: 384)
!577 = !DIDerivedType(tag: DW_TAG_member, name: "noexpand", scope: !556, file: !551, line: 926, baseType: !157, size: 32, offset: 416)
!578 = !DIDerivedType(tag: DW_TAG_member, name: "signature", scope: !556, file: !551, line: 928, baseType: !155, size: 32, offset: 448)
!579 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !552, file: !551, line: 939, baseType: !191, size: 64, offset: 64)
!580 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !552, file: !551, line: 940, baseType: !191, size: 64, offset: 128)
!581 = !DIDerivedType(tag: DW_TAG_member, name: "hh_prev", scope: !552, file: !551, line: 941, baseType: !564, size: 64, offset: 192)
!582 = !DIDerivedType(tag: DW_TAG_member, name: "hh_next", scope: !552, file: !551, line: 942, baseType: !564, size: 64, offset: 256)
!583 = !DIDerivedType(tag: DW_TAG_member, name: "key", scope: !552, file: !551, line: 943, baseType: !191, size: 64, offset: 320)
!584 = !DIDerivedType(tag: DW_TAG_member, name: "keylen", scope: !552, file: !551, line: 944, baseType: !157, size: 32, offset: 384)
!585 = !DIDerivedType(tag: DW_TAG_member, name: "hashv", scope: !552, file: !551, line: 945, baseType: !157, size: 32, offset: 416)
!586 = !DIDerivedType(tag: DW_TAG_member, name: "psk_id", scope: !337, file: !55, line: 270, baseType: !540, size: 64, offset: 2048)
!587 = !DIDerivedType(tag: DW_TAG_member, name: "dest_ids", scope: !324, file: !55, line: 367, baseType: !588, size: 64, offset: 384)
!588 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !124, size: 64)
!589 = !DIDerivedType(tag: DW_TAG_member, name: "dest_id_count", scope: !324, file: !55, line: 368, baseType: !116, size: 32, offset: 448)
!590 = !DIDerivedType(tag: DW_TAG_member, name: "ref_count", scope: !324, file: !55, line: 369, baseType: !116, size: 32, offset: 480)
!591 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !324, file: !55, line: 370, baseType: !124, size: 64, offset: 512)
!592 = !DIDerivedType(tag: DW_TAG_member, name: "properties", scope: !324, file: !55, line: 371, baseType: !178, size: 64, offset: 576)
!593 = !DIDerivedType(tag: DW_TAG_member, name: "payload", scope: !324, file: !55, line: 372, baseType: !594, size: 64, offset: 640)
!594 = !DIDerivedType(tag: DW_TAG_typedef, name: "mosquitto__payload_uhpa", file: !55, line: 125, baseType: !595)
!595 = distinct !DICompositeType(tag: DW_TAG_union_type, file: !55, line: 122, size: 64, elements: !596)
!596 = !{!597, !598}
!597 = !DIDerivedType(tag: DW_TAG_member, name: "ptr", scope: !595, file: !55, line: 123, baseType: !191, size: 64)
!598 = !DIDerivedType(tag: DW_TAG_member, name: "array", scope: !595, file: !55, line: 124, baseType: !599, size: 64)
!599 = !DICompositeType(tag: DW_TAG_array_type, baseType: !125, size: 64, elements: !600)
!600 = !{!601}
!601 = !DISubrange(count: 8)
!602 = !DIDerivedType(tag: DW_TAG_member, name: "message_expiry_time", scope: !324, file: !55, line: 373, baseType: !138, size: 64, offset: 704)
!603 = !DIDerivedType(tag: DW_TAG_member, name: "payloadlen", scope: !324, file: !55, line: 374, baseType: !155, size: 32, offset: 768)
!604 = !DIDerivedType(tag: DW_TAG_member, name: "source_mid", scope: !324, file: !55, line: 375, baseType: !130, size: 16, offset: 800)
!605 = !DIDerivedType(tag: DW_TAG_member, name: "mid", scope: !324, file: !55, line: 376, baseType: !130, size: 16, offset: 816)
!606 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !324, file: !55, line: 377, baseType: !149, size: 8, offset: 832)
!607 = !DIDerivedType(tag: DW_TAG_member, name: "retain", scope: !324, file: !55, line: 378, baseType: !184, size: 8, offset: 840)
!608 = !DIDerivedType(tag: DW_TAG_member, name: "origin", scope: !324, file: !55, line: 379, baseType: !149, size: 8, offset: 848)
!609 = !DIDerivedType(tag: DW_TAG_member, name: "properties", scope: !318, file: !55, line: 386, baseType: !178, size: 64, offset: 192)
!610 = !DIDerivedType(tag: DW_TAG_member, name: "timestamp", scope: !318, file: !55, line: 387, baseType: !138, size: 64, offset: 256)
!611 = !DIDerivedType(tag: DW_TAG_member, name: "mid", scope: !318, file: !55, line: 388, baseType: !130, size: 16, offset: 320)
!612 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !318, file: !55, line: 389, baseType: !149, size: 8, offset: 336)
!613 = !DIDerivedType(tag: DW_TAG_member, name: "retain", scope: !318, file: !55, line: 390, baseType: !184, size: 8, offset: 344)
!614 = !DIDerivedType(tag: DW_TAG_member, name: "direction", scope: !318, file: !55, line: 391, baseType: !72, size: 32, offset: 352)
!615 = !DIDerivedType(tag: DW_TAG_member, name: "state", scope: !318, file: !55, line: 392, baseType: !36, size: 32, offset: 384)
!616 = !DIDerivedType(tag: DW_TAG_member, name: "dup", scope: !318, file: !55, line: 393, baseType: !184, size: 8, offset: 416)
!617 = !DIDerivedType(tag: DW_TAG_member, name: "queued", scope: !314, file: !6, line: 191, baseType: !317, size: 64, offset: 64)
!618 = !DIDerivedType(tag: DW_TAG_member, name: "msg_bytes", scope: !314, file: !6, line: 192, baseType: !332, size: 64, offset: 128)
!619 = !DIDerivedType(tag: DW_TAG_member, name: "msg_bytes12", scope: !314, file: !6, line: 193, baseType: !332, size: 64, offset: 192)
!620 = !DIDerivedType(tag: DW_TAG_member, name: "msg_count", scope: !314, file: !6, line: 194, baseType: !116, size: 32, offset: 256)
!621 = !DIDerivedType(tag: DW_TAG_member, name: "msg_count12", scope: !314, file: !6, line: 195, baseType: !116, size: 32, offset: 288)
!622 = !DIDerivedType(tag: DW_TAG_member, name: "inflight_quota", scope: !314, file: !6, line: 203, baseType: !116, size: 32, offset: 320)
!623 = !DIDerivedType(tag: DW_TAG_member, name: "inflight_maximum", scope: !314, file: !6, line: 204, baseType: !130, size: 16, offset: 352)
!624 = !DIDerivedType(tag: DW_TAG_member, name: "msgs_out", scope: !118, file: !6, line: 279, baseType: !314, size: 384, offset: 3072)
!625 = !DIDerivedType(tag: DW_TAG_member, name: "acl_list", scope: !118, file: !6, line: 280, baseType: !374, size: 64, offset: 3456)
!626 = !DIDerivedType(tag: DW_TAG_member, name: "listener", scope: !118, file: !6, line: 281, baseType: !336, size: 64, offset: 3520)
!627 = !DIDerivedType(tag: DW_TAG_member, name: "out_packet_last", scope: !118, file: !6, line: 282, baseType: !153, size: 64, offset: 3584)
!628 = !DIDerivedType(tag: DW_TAG_member, name: "subs", scope: !118, file: !6, line: 283, baseType: !629, size: 64, offset: 3648)
!629 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !630, size: 64)
!630 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !631, size: 64)
!631 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__subhier", file: !55, line: 343, size: 896, elements: !632)
!632 = !{!633, !634, !635, !636, !647, !654, !655, !656}
!633 = !DIDerivedType(tag: DW_TAG_member, name: "hh", scope: !631, file: !55, line: 344, baseType: !550, size: 448)
!634 = !DIDerivedType(tag: DW_TAG_member, name: "parent", scope: !631, file: !55, line: 345, baseType: !630, size: 64, offset: 448)
!635 = !DIDerivedType(tag: DW_TAG_member, name: "children", scope: !631, file: !55, line: 346, baseType: !630, size: 64, offset: 512)
!636 = !DIDerivedType(tag: DW_TAG_member, name: "subs", scope: !631, file: !55, line: 347, baseType: !637, size: 64, offset: 576)
!637 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !638, size: 64)
!638 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__subleaf", file: !55, line: 320, size: 256, elements: !639)
!639 = !{!640, !641, !642, !643, !644, !645, !646}
!640 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !638, file: !55, line: 321, baseType: !637, size: 64)
!641 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !638, file: !55, line: 322, baseType: !637, size: 64, offset: 64)
!642 = !DIDerivedType(tag: DW_TAG_member, name: "context", scope: !638, file: !55, line: 323, baseType: !117, size: 64, offset: 128)
!643 = !DIDerivedType(tag: DW_TAG_member, name: "identifier", scope: !638, file: !55, line: 324, baseType: !155, size: 32, offset: 192)
!644 = !DIDerivedType(tag: DW_TAG_member, name: "qos", scope: !638, file: !55, line: 325, baseType: !149, size: 8, offset: 224)
!645 = !DIDerivedType(tag: DW_TAG_member, name: "no_local", scope: !638, file: !55, line: 326, baseType: !184, size: 8, offset: 232)
!646 = !DIDerivedType(tag: DW_TAG_member, name: "retain_as_published", scope: !638, file: !55, line: 327, baseType: !184, size: 8, offset: 240)
!647 = !DIDerivedType(tag: DW_TAG_member, name: "shared", scope: !631, file: !55, line: 348, baseType: !648, size: 64, offset: 640)
!648 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !649, size: 64)
!649 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__subshared", file: !55, line: 337, size: 576, elements: !650)
!650 = !{!651, !652, !653}
!651 = !DIDerivedType(tag: DW_TAG_member, name: "hh", scope: !649, file: !55, line: 338, baseType: !550, size: 448)
!652 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !649, file: !55, line: 339, baseType: !124, size: 64, offset: 448)
!653 = !DIDerivedType(tag: DW_TAG_member, name: "subs", scope: !649, file: !55, line: 340, baseType: !637, size: 64, offset: 512)
!654 = !DIDerivedType(tag: DW_TAG_member, name: "retained", scope: !631, file: !55, line: 349, baseType: !323, size: 64, offset: 704)
!655 = !DIDerivedType(tag: DW_TAG_member, name: "topic", scope: !631, file: !55, line: 350, baseType: !124, size: 64, offset: 768)
!656 = !DIDerivedType(tag: DW_TAG_member, name: "topic_len", scope: !631, file: !55, line: 351, baseType: !130, size: 16, offset: 832)
!657 = !DIDerivedType(tag: DW_TAG_member, name: "shared_subs", scope: !118, file: !6, line: 284, baseType: !658, size: 64, offset: 3712)
!658 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !659, size: 64)
!659 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !660, size: 64)
!660 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__subshared_ref", file: !55, line: 331, size: 128, elements: !661)
!661 = !{!662, !663}
!662 = !DIDerivedType(tag: DW_TAG_member, name: "hier", scope: !660, file: !55, line: 332, baseType: !630, size: 64)
!663 = !DIDerivedType(tag: DW_TAG_member, name: "shared", scope: !660, file: !55, line: 333, baseType: !648, size: 64, offset: 64)
!664 = !DIDerivedType(tag: DW_TAG_member, name: "auth_method", scope: !118, file: !6, line: 285, baseType: !124, size: 64, offset: 3776)
!665 = !DIDerivedType(tag: DW_TAG_member, name: "sub_count", scope: !118, file: !6, line: 286, baseType: !116, size: 32, offset: 3840)
!666 = !DIDerivedType(tag: DW_TAG_member, name: "shared_sub_count", scope: !118, file: !6, line: 287, baseType: !116, size: 32, offset: 3872)
!667 = !DIDerivedType(tag: DW_TAG_member, name: "pollfd_index", scope: !118, file: !6, line: 288, baseType: !116, size: 32, offset: 3904)
!668 = !DIDerivedType(tag: DW_TAG_member, name: "ws_want_write", scope: !118, file: !6, line: 297, baseType: !184, size: 8, offset: 3936)
!669 = !DIDerivedType(tag: DW_TAG_member, name: "assigned_id", scope: !118, file: !6, line: 298, baseType: !184, size: 8, offset: 3944)
!670 = !DIDerivedType(tag: DW_TAG_member, name: "maximum_qos", scope: !118, file: !6, line: 338, baseType: !149, size: 8, offset: 3952)
!671 = !DIDerivedType(tag: DW_TAG_member, name: "hh_id", scope: !118, file: !6, line: 341, baseType: !550, size: 448, offset: 3968)
!672 = !DIDerivedType(tag: DW_TAG_member, name: "hh_sock", scope: !118, file: !6, line: 342, baseType: !550, size: 448, offset: 4416)
!673 = !DIDerivedType(tag: DW_TAG_member, name: "for_free_next", scope: !118, file: !6, line: 343, baseType: !117, size: 64, offset: 4864)
!674 = !DIDerivedType(tag: DW_TAG_member, name: "expiry_list_item", scope: !118, file: !6, line: 344, baseType: !675, size: 64, offset: 4928)
!675 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !676, size: 64)
!676 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "session_expiry_list", file: !6, line: 144, size: 192, elements: !677)
!677 = !{!678, !679, !680}
!678 = !DIDerivedType(tag: DW_TAG_member, name: "context", scope: !676, file: !6, line: 145, baseType: !117, size: 64)
!679 = !DIDerivedType(tag: DW_TAG_member, name: "prev", scope: !676, file: !6, line: 146, baseType: !675, size: 64, offset: 64)
!680 = !DIDerivedType(tag: DW_TAG_member, name: "next", scope: !676, file: !6, line: 147, baseType: !675, size: 64, offset: 128)
!681 = !DIDerivedType(tag: DW_TAG_member, name: "events", scope: !118, file: !6, line: 347, baseType: !155, size: 32, offset: 4992)
!682 = !{!0, !683, !685, !687, !817}
!683 = !DIGlobalVariableExpression(var: !684, expr: !DIExpression())
!684 = distinct !DIGlobalVariable(name: "flag_db_backup", scope: !2, file: !3, line: 60, type: !184, isLocal: false, isDefinition: true)
!685 = !DIGlobalVariableExpression(var: !686, expr: !DIExpression())
!686 = distinct !DIGlobalVariable(name: "flag_tree_print", scope: !2, file: !3, line: 62, type: !184, isLocal: false, isDefinition: true)
!687 = !DIGlobalVariableExpression(var: !688, expr: !DIExpression())
!688 = distinct !DIGlobalVariable(name: "int_db", scope: !2, file: !3, line: 56, type: !689, isLocal: false, isDefinition: true)
!689 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_db", file: !55, line: 421, size: 1280, elements: !690)
!690 = !{!691, !692, !693, !694, !695, !696, !697, !698, !700, !703, !704, !711, !712, !713, !714, !715, !809, !810, !811, !812, !813, !814, !815, !816}
!691 = !DIDerivedType(tag: DW_TAG_member, name: "last_db_id", scope: !689, file: !55, line: 422, baseType: !329, size: 64)
!692 = !DIDerivedType(tag: DW_TAG_member, name: "subs", scope: !689, file: !55, line: 423, baseType: !630, size: 64, offset: 64)
!693 = !DIDerivedType(tag: DW_TAG_member, name: "unpwd", scope: !689, file: !55, line: 424, baseType: !540, size: 64, offset: 128)
!694 = !DIDerivedType(tag: DW_TAG_member, name: "psk_id", scope: !689, file: !55, line: 425, baseType: !540, size: 64, offset: 192)
!695 = !DIDerivedType(tag: DW_TAG_member, name: "contexts_by_id", scope: !689, file: !55, line: 426, baseType: !117, size: 64, offset: 256)
!696 = !DIDerivedType(tag: DW_TAG_member, name: "contexts_by_sock", scope: !689, file: !55, line: 427, baseType: !117, size: 64, offset: 320)
!697 = !DIDerivedType(tag: DW_TAG_member, name: "contexts_for_free", scope: !689, file: !55, line: 428, baseType: !117, size: 64, offset: 384)
!698 = !DIDerivedType(tag: DW_TAG_member, name: "bridges", scope: !689, file: !55, line: 430, baseType: !699, size: 64, offset: 448)
!699 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !117, size: 64)
!700 = !DIDerivedType(tag: DW_TAG_member, name: "clientid_index_hash", scope: !689, file: !55, line: 432, baseType: !701, size: 64, offset: 512)
!701 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !702, size: 64)
!702 = !DICompositeType(tag: DW_TAG_structure_type, name: "clientid__index_hash", file: !55, line: 432, flags: DIFlagFwdDecl)
!703 = !DIDerivedType(tag: DW_TAG_member, name: "msg_store", scope: !689, file: !55, line: 433, baseType: !323, size: 64, offset: 576)
!704 = !DIDerivedType(tag: DW_TAG_member, name: "msg_store_load", scope: !689, file: !55, line: 434, baseType: !705, size: 64, offset: 640)
!705 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !706, size: 64)
!706 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto_msg_store_load", file: !55, line: 354, size: 576, elements: !707)
!707 = !{!708, !709, !710}
!708 = !DIDerivedType(tag: DW_TAG_member, name: "hh", scope: !706, file: !55, line: 355, baseType: !550, size: 448)
!709 = !DIDerivedType(tag: DW_TAG_member, name: "db_id", scope: !706, file: !55, line: 356, baseType: !329, size: 64, offset: 448)
!710 = !DIDerivedType(tag: DW_TAG_member, name: "store", scope: !706, file: !55, line: 357, baseType: !323, size: 64, offset: 512)
!711 = !DIDerivedType(tag: DW_TAG_member, name: "bridge_count", scope: !689, file: !55, line: 436, baseType: !116, size: 32, offset: 704)
!712 = !DIDerivedType(tag: DW_TAG_member, name: "msg_store_count", scope: !689, file: !55, line: 438, baseType: !116, size: 32, offset: 736)
!713 = !DIDerivedType(tag: DW_TAG_member, name: "msg_store_bytes", scope: !689, file: !55, line: 439, baseType: !332, size: 64, offset: 768)
!714 = !DIDerivedType(tag: DW_TAG_member, name: "config_file", scope: !689, file: !55, line: 440, baseType: !124, size: 64, offset: 832)
!715 = !DIDerivedType(tag: DW_TAG_member, name: "config", scope: !689, file: !55, line: 441, baseType: !716, size: 64, offset: 896)
!716 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !717, size: 64)
!717 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "mosquitto__config", file: !55, line: 273, size: 4160, elements: !718)
!718 = !{!719, !720, !721, !722, !723, !724, !725, !726, !727, !728, !729, !730, !731, !732, !733, !734, !735, !789, !790, !791, !792, !793, !794, !795, !796, !797, !798, !799, !800, !801, !802, !803, !804, !805, !806, !807, !808}
!719 = !DIDerivedType(tag: DW_TAG_member, name: "allow_duplicate_messages", scope: !717, file: !55, line: 274, baseType: !184, size: 8)
!720 = !DIDerivedType(tag: DW_TAG_member, name: "autosave_interval", scope: !717, file: !55, line: 275, baseType: !116, size: 32, offset: 32)
!721 = !DIDerivedType(tag: DW_TAG_member, name: "autosave_on_changes", scope: !717, file: !55, line: 276, baseType: !184, size: 8, offset: 64)
!722 = !DIDerivedType(tag: DW_TAG_member, name: "check_retain_source", scope: !717, file: !55, line: 277, baseType: !184, size: 8, offset: 72)
!723 = !DIDerivedType(tag: DW_TAG_member, name: "clientid_prefixes", scope: !717, file: !55, line: 278, baseType: !124, size: 64, offset: 128)
!724 = !DIDerivedType(tag: DW_TAG_member, name: "connection_messages", scope: !717, file: !55, line: 279, baseType: !184, size: 8, offset: 192)
!725 = !DIDerivedType(tag: DW_TAG_member, name: "daemon", scope: !717, file: !55, line: 280, baseType: !184, size: 8, offset: 200)
!726 = !DIDerivedType(tag: DW_TAG_member, name: "default_listener", scope: !717, file: !55, line: 281, baseType: !337, size: 2112, offset: 256)
!727 = !DIDerivedType(tag: DW_TAG_member, name: "listeners", scope: !717, file: !55, line: 282, baseType: !336, size: 64, offset: 2368)
!728 = !DIDerivedType(tag: DW_TAG_member, name: "listener_count", scope: !717, file: !55, line: 283, baseType: !116, size: 32, offset: 2432)
!729 = !DIDerivedType(tag: DW_TAG_member, name: "log_dest", scope: !717, file: !55, line: 284, baseType: !116, size: 32, offset: 2464)
!730 = !DIDerivedType(tag: DW_TAG_member, name: "log_facility", scope: !717, file: !55, line: 285, baseType: !116, size: 32, offset: 2496)
!731 = !DIDerivedType(tag: DW_TAG_member, name: "log_type", scope: !717, file: !55, line: 286, baseType: !116, size: 32, offset: 2528)
!732 = !DIDerivedType(tag: DW_TAG_member, name: "log_timestamp", scope: !717, file: !55, line: 287, baseType: !184, size: 8, offset: 2560)
!733 = !DIDerivedType(tag: DW_TAG_member, name: "log_timestamp_format", scope: !717, file: !55, line: 288, baseType: !124, size: 64, offset: 2624)
!734 = !DIDerivedType(tag: DW_TAG_member, name: "log_file", scope: !717, file: !55, line: 289, baseType: !124, size: 64, offset: 2688)
!735 = !DIDerivedType(tag: DW_TAG_member, name: "log_fptr", scope: !717, file: !55, line: 290, baseType: !736, size: 64, offset: 2752)
!736 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !737, size: 64)
!737 = !DIDerivedType(tag: DW_TAG_typedef, name: "FILE", file: !738, line: 7, baseType: !739)
!738 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/types/FILE.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!739 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "_IO_FILE", file: !740, line: 245, size: 1728, elements: !741)
!740 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/libio.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!741 = !{!742, !743, !744, !745, !746, !747, !748, !749, !750, !751, !752, !753, !754, !762, !763, !764, !765, !767, !768, !769, !773, !776, !778, !779, !780, !781, !782, !784, !785}
!742 = !DIDerivedType(tag: DW_TAG_member, name: "_flags", scope: !739, file: !740, line: 246, baseType: !116, size: 32)
!743 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_read_ptr", scope: !739, file: !740, line: 251, baseType: !124, size: 64, offset: 64)
!744 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_read_end", scope: !739, file: !740, line: 252, baseType: !124, size: 64, offset: 128)
!745 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_read_base", scope: !739, file: !740, line: 253, baseType: !124, size: 64, offset: 192)
!746 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_write_base", scope: !739, file: !740, line: 254, baseType: !124, size: 64, offset: 256)
!747 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_write_ptr", scope: !739, file: !740, line: 255, baseType: !124, size: 64, offset: 320)
!748 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_write_end", scope: !739, file: !740, line: 256, baseType: !124, size: 64, offset: 384)
!749 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_buf_base", scope: !739, file: !740, line: 257, baseType: !124, size: 64, offset: 448)
!750 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_buf_end", scope: !739, file: !740, line: 258, baseType: !124, size: 64, offset: 512)
!751 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_save_base", scope: !739, file: !740, line: 260, baseType: !124, size: 64, offset: 576)
!752 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_backup_base", scope: !739, file: !740, line: 261, baseType: !124, size: 64, offset: 640)
!753 = !DIDerivedType(tag: DW_TAG_member, name: "_IO_save_end", scope: !739, file: !740, line: 262, baseType: !124, size: 64, offset: 704)
!754 = !DIDerivedType(tag: DW_TAG_member, name: "_markers", scope: !739, file: !740, line: 264, baseType: !755, size: 64, offset: 768)
!755 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !756, size: 64)
!756 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "_IO_marker", file: !740, line: 160, size: 192, elements: !757)
!757 = !{!758, !759, !761}
!758 = !DIDerivedType(tag: DW_TAG_member, name: "_next", scope: !756, file: !740, line: 161, baseType: !755, size: 64)
!759 = !DIDerivedType(tag: DW_TAG_member, name: "_sbuf", scope: !756, file: !740, line: 162, baseType: !760, size: 64, offset: 64)
!760 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !739, size: 64)
!761 = !DIDerivedType(tag: DW_TAG_member, name: "_pos", scope: !756, file: !740, line: 166, baseType: !116, size: 32, offset: 128)
!762 = !DIDerivedType(tag: DW_TAG_member, name: "_chain", scope: !739, file: !740, line: 266, baseType: !760, size: 64, offset: 832)
!763 = !DIDerivedType(tag: DW_TAG_member, name: "_fileno", scope: !739, file: !740, line: 268, baseType: !116, size: 32, offset: 896)
!764 = !DIDerivedType(tag: DW_TAG_member, name: "_flags2", scope: !739, file: !740, line: 272, baseType: !116, size: 32, offset: 928)
!765 = !DIDerivedType(tag: DW_TAG_member, name: "_old_offset", scope: !739, file: !740, line: 274, baseType: !766, size: 64, offset: 960)
!766 = !DIDerivedType(tag: DW_TAG_typedef, name: "__off_t", file: !133, line: 140, baseType: !141)
!767 = !DIDerivedType(tag: DW_TAG_member, name: "_cur_column", scope: !739, file: !740, line: 278, baseType: !134, size: 16, offset: 1024)
!768 = !DIDerivedType(tag: DW_TAG_member, name: "_vtable_offset", scope: !739, file: !740, line: 279, baseType: !168, size: 8, offset: 1040)
!769 = !DIDerivedType(tag: DW_TAG_member, name: "_shortbuf", scope: !739, file: !740, line: 280, baseType: !770, size: 8, offset: 1048)
!770 = !DICompositeType(tag: DW_TAG_array_type, baseType: !125, size: 8, elements: !771)
!771 = !{!772}
!772 = !DISubrange(count: 1)
!773 = !DIDerivedType(tag: DW_TAG_member, name: "_lock", scope: !739, file: !740, line: 284, baseType: !774, size: 64, offset: 1088)
!774 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !775, size: 64)
!775 = !DIDerivedType(tag: DW_TAG_typedef, name: "_IO_lock_t", file: !740, line: 154, baseType: null)
!776 = !DIDerivedType(tag: DW_TAG_member, name: "_offset", scope: !739, file: !740, line: 293, baseType: !777, size: 64, offset: 1152)
!777 = !DIDerivedType(tag: DW_TAG_typedef, name: "__off64_t", file: !133, line: 141, baseType: !141)
!778 = !DIDerivedType(tag: DW_TAG_member, name: "__pad1", scope: !739, file: !740, line: 301, baseType: !191, size: 64, offset: 1216)
!779 = !DIDerivedType(tag: DW_TAG_member, name: "__pad2", scope: !739, file: !740, line: 302, baseType: !191, size: 64, offset: 1280)
!780 = !DIDerivedType(tag: DW_TAG_member, name: "__pad3", scope: !739, file: !740, line: 303, baseType: !191, size: 64, offset: 1344)
!781 = !DIDerivedType(tag: DW_TAG_member, name: "__pad4", scope: !739, file: !740, line: 304, baseType: !191, size: 64, offset: 1408)
!782 = !DIDerivedType(tag: DW_TAG_member, name: "__pad5", scope: !739, file: !740, line: 306, baseType: !783, size: 64, offset: 1472)
!783 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !573, line: 62, baseType: !332)
!784 = !DIDerivedType(tag: DW_TAG_member, name: "_mode", scope: !739, file: !740, line: 307, baseType: !116, size: 32, offset: 1536)
!785 = !DIDerivedType(tag: DW_TAG_member, name: "_unused2", scope: !739, file: !740, line: 309, baseType: !786, size: 160, offset: 1568)
!786 = !DICompositeType(tag: DW_TAG_array_type, baseType: !125, size: 160, elements: !787)
!787 = !{!788}
!788 = !DISubrange(count: 20)
!789 = !DIDerivedType(tag: DW_TAG_member, name: "max_inflight_messages", scope: !717, file: !55, line: 291, baseType: !130, size: 16, offset: 2816)
!790 = !DIDerivedType(tag: DW_TAG_member, name: "max_keepalive", scope: !717, file: !55, line: 292, baseType: !130, size: 16, offset: 2832)
!791 = !DIDerivedType(tag: DW_TAG_member, name: "max_packet_size", scope: !717, file: !55, line: 293, baseType: !155, size: 32, offset: 2848)
!792 = !DIDerivedType(tag: DW_TAG_member, name: "message_size_limit", scope: !717, file: !55, line: 294, baseType: !155, size: 32, offset: 2880)
!793 = !DIDerivedType(tag: DW_TAG_member, name: "persistence", scope: !717, file: !55, line: 295, baseType: !184, size: 8, offset: 2912)
!794 = !DIDerivedType(tag: DW_TAG_member, name: "persistence_location", scope: !717, file: !55, line: 296, baseType: !124, size: 64, offset: 2944)
!795 = !DIDerivedType(tag: DW_TAG_member, name: "persistence_file", scope: !717, file: !55, line: 297, baseType: !124, size: 64, offset: 3008)
!796 = !DIDerivedType(tag: DW_TAG_member, name: "persistence_filepath", scope: !717, file: !55, line: 298, baseType: !124, size: 64, offset: 3072)
!797 = !DIDerivedType(tag: DW_TAG_member, name: "persistent_client_expiration", scope: !717, file: !55, line: 299, baseType: !138, size: 64, offset: 3136)
!798 = !DIDerivedType(tag: DW_TAG_member, name: "pid_file", scope: !717, file: !55, line: 300, baseType: !124, size: 64, offset: 3200)
!799 = !DIDerivedType(tag: DW_TAG_member, name: "queue_qos0_messages", scope: !717, file: !55, line: 301, baseType: !184, size: 8, offset: 3264)
!800 = !DIDerivedType(tag: DW_TAG_member, name: "per_listener_settings", scope: !717, file: !55, line: 302, baseType: !184, size: 8, offset: 3272)
!801 = !DIDerivedType(tag: DW_TAG_member, name: "retain_available", scope: !717, file: !55, line: 303, baseType: !184, size: 8, offset: 3280)
!802 = !DIDerivedType(tag: DW_TAG_member, name: "set_tcp_nodelay", scope: !717, file: !55, line: 304, baseType: !184, size: 8, offset: 3288)
!803 = !DIDerivedType(tag: DW_TAG_member, name: "sys_interval", scope: !717, file: !55, line: 305, baseType: !116, size: 32, offset: 3296)
!804 = !DIDerivedType(tag: DW_TAG_member, name: "upgrade_outgoing_qos", scope: !717, file: !55, line: 306, baseType: !184, size: 8, offset: 3328)
!805 = !DIDerivedType(tag: DW_TAG_member, name: "user", scope: !717, file: !55, line: 307, baseType: !124, size: 64, offset: 3392)
!806 = !DIDerivedType(tag: DW_TAG_member, name: "bridges", scope: !717, file: !55, line: 314, baseType: !251, size: 64, offset: 3456)
!807 = !DIDerivedType(tag: DW_TAG_member, name: "bridge_count", scope: !717, file: !55, line: 315, baseType: !116, size: 32, offset: 3520)
!808 = !DIDerivedType(tag: DW_TAG_member, name: "security_options", scope: !717, file: !55, line: 317, baseType: !371, size: 576, offset: 3584)
!809 = !DIDerivedType(tag: DW_TAG_member, name: "auth_plugin_count", scope: !689, file: !55, line: 442, baseType: !116, size: 32, offset: 960)
!810 = !DIDerivedType(tag: DW_TAG_member, name: "verbose", scope: !689, file: !55, line: 443, baseType: !184, size: 8, offset: 992)
!811 = !DIDerivedType(tag: DW_TAG_member, name: "subscription_count", scope: !689, file: !55, line: 445, baseType: !116, size: 32, offset: 1024)
!812 = !DIDerivedType(tag: DW_TAG_member, name: "shared_subscription_count", scope: !689, file: !55, line: 446, baseType: !116, size: 32, offset: 1056)
!813 = !DIDerivedType(tag: DW_TAG_member, name: "retained_count", scope: !689, file: !55, line: 447, baseType: !116, size: 32, offset: 1088)
!814 = !DIDerivedType(tag: DW_TAG_member, name: "persistence_changes", scope: !689, file: !55, line: 449, baseType: !116, size: 32, offset: 1120)
!815 = !DIDerivedType(tag: DW_TAG_member, name: "ll_for_free", scope: !689, file: !55, line: 450, baseType: !117, size: 64, offset: 1152)
!816 = !DIDerivedType(tag: DW_TAG_member, name: "epollfd", scope: !689, file: !55, line: 452, baseType: !116, size: 32, offset: 1216)
!817 = !DIGlobalVariableExpression(var: !818, expr: !DIExpression())
!818 = distinct !DIGlobalVariable(name: "run", scope: !2, file: !3, line: 63, type: !116, isLocal: false, isDefinition: true)
!819 = distinct !DICompileUnit(language: DW_LANG_C89, file: !820, producer: "clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !821)
!820 = !DIFile(filename: "/home/ryan/git/klee/runtime/FreeStanding/memset.c", directory: "/home/ryan/git/klee/build/runtime/FreeStanding")
!821 = !{}
!822 = !{i32 2, !"Dwarf Version", i32 4}
!823 = !{i32 2, !"Debug Info Version", i32 3}
!824 = !{i32 1, !"wchar_size", i32 4}
!825 = !{!"clang version 6.0.0-1ubuntu2 (tags/RELEASE_600/final)"}
!826 = distinct !DISubprogram(name: "mosquitto__get_db", scope: !3, file: !3, line: 77, type: !827, isLocal: false, isDefinition: true, scopeLine: 78, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !821)
!827 = !DISubroutineType(types: !828)
!828 = !{!829}
!829 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !689, size: 64)
!830 = !DILocation(line: 79, column: 2, scope: !826)
!831 = distinct !DISubprogram(name: "drop_privileges", scope: !3, file: !3, line: 90, type: !832, isLocal: false, isDefinition: true, scopeLine: 91, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !834)
!832 = !DISubroutineType(types: !833)
!833 = !{!116, !716, !184}
!834 = !{!835, !836, !837, !851, !852, !853}
!835 = !DILocalVariable(name: "config", arg: 1, scope: !831, file: !3, line: 90, type: !716)
!836 = !DILocalVariable(name: "temporary", arg: 2, scope: !831, file: !3, line: 90, type: !184)
!837 = !DILocalVariable(name: "pwd", scope: !831, file: !3, line: 93, type: !838)
!838 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !839, size: 64)
!839 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "passwd", file: !840, line: 49, size: 384, elements: !841)
!840 = !DIFile(filename: "/usr/include/pwd.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!841 = !{!842, !843, !844, !846, !848, !849, !850}
!842 = !DIDerivedType(tag: DW_TAG_member, name: "pw_name", scope: !839, file: !840, line: 51, baseType: !124, size: 64)
!843 = !DIDerivedType(tag: DW_TAG_member, name: "pw_passwd", scope: !839, file: !840, line: 52, baseType: !124, size: 64, offset: 64)
!844 = !DIDerivedType(tag: DW_TAG_member, name: "pw_uid", scope: !839, file: !840, line: 53, baseType: !845, size: 32, offset: 128)
!845 = !DIDerivedType(tag: DW_TAG_typedef, name: "__uid_t", file: !133, line: 134, baseType: !157)
!846 = !DIDerivedType(tag: DW_TAG_member, name: "pw_gid", scope: !839, file: !840, line: 54, baseType: !847, size: 32, offset: 160)
!847 = !DIDerivedType(tag: DW_TAG_typedef, name: "__gid_t", file: !133, line: 135, baseType: !157)
!848 = !DIDerivedType(tag: DW_TAG_member, name: "pw_gecos", scope: !839, file: !840, line: 55, baseType: !124, size: 64, offset: 192)
!849 = !DIDerivedType(tag: DW_TAG_member, name: "pw_dir", scope: !839, file: !840, line: 56, baseType: !124, size: 64, offset: 256)
!850 = !DIDerivedType(tag: DW_TAG_member, name: "pw_shell", scope: !839, file: !840, line: 57, baseType: !124, size: 64, offset: 320)
!851 = !DILocalVariable(name: "err", scope: !831, file: !3, line: 94, type: !124)
!852 = !DILocalVariable(name: "rc", scope: !831, file: !3, line: 95, type: !116)
!853 = !DILocalVariable(name: "snap", scope: !831, file: !3, line: 97, type: !442)
!854 = !DILocation(line: 90, column: 47, scope: !831)
!855 = !DILocation(line: 90, column: 60, scope: !831)
!856 = !DILocation(line: 97, column: 21, scope: !831)
!857 = !DILocation(line: 97, column: 14, scope: !831)
!858 = !DILocation(line: 98, column: 5, scope: !859)
!859 = distinct !DILexicalBlock(scope: !831, file: !3, line: 98, column: 5)
!860 = !DILocation(line: 98, column: 10, scope: !859)
!861 = !DILocation(line: 98, column: 14, scope: !859)
!862 = !DILocation(line: 98, column: 5, scope: !831)
!863 = !DILocation(line: 103, column: 5, scope: !864)
!864 = distinct !DILexicalBlock(scope: !831, file: !3, line: 103, column: 5)
!865 = !DILocation(line: 103, column: 15, scope: !864)
!866 = !DILocation(line: 103, column: 5, scope: !831)
!867 = !DILocation(line: 104, column: 14, scope: !868)
!868 = distinct !DILexicalBlock(scope: !869, file: !3, line: 104, column: 6)
!869 = distinct !DILexicalBlock(scope: !864, file: !3, line: 103, column: 20)
!870 = !{!871, !876, i64 424}
!871 = !{!"mosquitto__config", !872, i64 0, !875, i64 4, !872, i64 8, !872, i64 9, !876, i64 16, !872, i64 24, !872, i64 25, !877, i64 32, !876, i64 296, !875, i64 304, !875, i64 308, !875, i64 312, !875, i64 316, !872, i64 320, !876, i64 328, !876, i64 336, !876, i64 344, !878, i64 352, !878, i64 354, !875, i64 356, !875, i64 360, !872, i64 364, !876, i64 368, !876, i64 376, !876, i64 384, !880, i64 392, !876, i64 400, !872, i64 408, !872, i64 409, !872, i64 410, !872, i64 411, !875, i64 412, !872, i64 416, !876, i64 424, !876, i64 432, !875, i64 440, !879, i64 448}
!872 = !{!"_Bool", !873, i64 0}
!873 = !{!"omnipotent char", !874, i64 0}
!874 = !{!"Simple C/C++ TBAA"}
!875 = !{!"int", !873, i64 0}
!876 = !{!"any pointer", !873, i64 0}
!877 = !{!"mosquitto__listener", !875, i64 0, !878, i64 4, !876, i64 8, !876, i64 16, !875, i64 24, !876, i64 32, !876, i64 40, !875, i64 48, !875, i64 52, !873, i64 56, !875, i64 60, !872, i64 64, !873, i64 65, !878, i64 66, !876, i64 72, !876, i64 80, !876, i64 88, !876, i64 96, !876, i64 104, !876, i64 112, !876, i64 120, !876, i64 128, !876, i64 136, !876, i64 144, !876, i64 152, !876, i64 160, !872, i64 168, !872, i64 169, !872, i64 170, !873, i64 172, !879, i64 176, !876, i64 248, !876, i64 256}
!878 = !{!"short", !873, i64 0}
!879 = !{!"mosquitto__security_options", !876, i64 0, !876, i64 8, !876, i64 16, !876, i64 24, !876, i64 32, !876, i64 40, !875, i64 48, !873, i64 52, !872, i64 53, !876, i64 56, !875, i64 64}
!880 = !{!"long", !873, i64 0}
!881 = !DILocation(line: 104, column: 6, scope: !868)
!882 = !DILocation(line: 104, column: 19, scope: !868)
!883 = !DILocation(line: 104, column: 22, scope: !868)
!884 = !DILocation(line: 104, column: 6, scope: !869)
!885 = !DILocation(line: 105, column: 10, scope: !886)
!886 = distinct !DILexicalBlock(scope: !868, file: !3, line: 104, column: 51)
!887 = !DILocation(line: 93, column: 17, scope: !831)
!888 = !DILocation(line: 106, column: 8, scope: !889)
!889 = distinct !DILexicalBlock(scope: !886, file: !3, line: 106, column: 7)
!890 = !DILocation(line: 106, column: 7, scope: !886)
!891 = !DILocation(line: 107, column: 5, scope: !892)
!892 = distinct !DILexicalBlock(scope: !889, file: !3, line: 106, column: 12)
!893 = !DILocation(line: 108, column: 5, scope: !892)
!894 = !DILocation(line: 110, column: 37, scope: !895)
!895 = distinct !DILexicalBlock(scope: !886, file: !3, line: 110, column: 7)
!896 = !{!897, !875, i64 20}
!897 = !{!"passwd", !876, i64 0, !876, i64 8, !875, i64 16, !875, i64 20, !876, i64 24, !876, i64 32, !876, i64 40}
!898 = !DILocation(line: 110, column: 7, scope: !895)
!899 = !DILocation(line: 110, column: 45, scope: !895)
!900 = !DILocation(line: 110, column: 7, scope: !886)
!901 = !DILocation(line: 111, column: 20, scope: !902)
!902 = distinct !DILexicalBlock(scope: !895, file: !3, line: 110, column: 51)
!903 = !{!875, !875, i64 0}
!904 = !DILocation(line: 111, column: 11, scope: !902)
!905 = !DILocation(line: 94, column: 8, scope: !831)
!906 = !DILocation(line: 112, column: 5, scope: !902)
!907 = !DILocation(line: 113, column: 5, scope: !902)
!908 = !DILocation(line: 115, column: 7, scope: !886)
!909 = !DILocation(line: 116, column: 10, scope: !910)
!910 = distinct !DILexicalBlock(scope: !911, file: !3, line: 115, column: 17)
!911 = distinct !DILexicalBlock(scope: !886, file: !3, line: 115, column: 7)
!912 = !DILocation(line: 95, column: 6, scope: !831)
!913 = !DILocation(line: 117, column: 4, scope: !910)
!914 = !DILocation(line: 118, column: 10, scope: !915)
!915 = distinct !DILexicalBlock(scope: !911, file: !3, line: 117, column: 9)
!916 = !DILocation(line: 120, column: 10, scope: !917)
!917 = distinct !DILexicalBlock(scope: !886, file: !3, line: 120, column: 7)
!918 = !DILocation(line: 120, column: 7, scope: !886)
!919 = !DILocation(line: 121, column: 20, scope: !920)
!920 = distinct !DILexicalBlock(scope: !917, file: !3, line: 120, column: 16)
!921 = !DILocation(line: 121, column: 11, scope: !920)
!922 = !DILocation(line: 122, column: 5, scope: !920)
!923 = !DILocation(line: 123, column: 5, scope: !920)
!924 = !{!897, !875, i64 16}
!925 = !DILocation(line: 125, column: 7, scope: !886)
!926 = !DILocation(line: 126, column: 10, scope: !927)
!927 = distinct !DILexicalBlock(scope: !928, file: !3, line: 125, column: 17)
!928 = distinct !DILexicalBlock(scope: !886, file: !3, line: 125, column: 7)
!929 = !DILocation(line: 127, column: 4, scope: !927)
!930 = !DILocation(line: 128, column: 10, scope: !931)
!931 = distinct !DILexicalBlock(scope: !928, file: !3, line: 127, column: 9)
!932 = !DILocation(line: 130, column: 10, scope: !933)
!933 = distinct !DILexicalBlock(scope: !886, file: !3, line: 130, column: 7)
!934 = !DILocation(line: 130, column: 7, scope: !886)
!935 = !DILocation(line: 131, column: 20, scope: !936)
!936 = distinct !DILexicalBlock(scope: !933, file: !3, line: 130, column: 16)
!937 = !DILocation(line: 131, column: 11, scope: !936)
!938 = !DILocation(line: 132, column: 5, scope: !936)
!939 = !DILocation(line: 133, column: 5, scope: !936)
!940 = !DILocation(line: 136, column: 6, scope: !941)
!941 = distinct !DILexicalBlock(scope: !869, file: !3, line: 136, column: 6)
!942 = !DILocation(line: 136, column: 16, scope: !941)
!943 = !DILocation(line: 136, column: 21, scope: !941)
!944 = !DILocation(line: 136, column: 24, scope: !941)
!945 = !DILocation(line: 136, column: 34, scope: !941)
!946 = !DILocation(line: 136, column: 6, scope: !869)
!947 = !DILocation(line: 137, column: 4, scope: !948)
!948 = distinct !DILexicalBlock(scope: !941, file: !3, line: 136, column: 39)
!949 = !DILocation(line: 138, column: 3, scope: !948)
!950 = !DILocation(line: 142, column: 1, scope: !831)
!951 = distinct !DISubprogram(name: "restore_privileges", scope: !3, file: !3, line: 144, type: !413, isLocal: false, isDefinition: true, scopeLine: 145, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !952)
!952 = !{!953, !954}
!953 = !DILocalVariable(name: "err", scope: !951, file: !3, line: 147, type: !124)
!954 = !DILocalVariable(name: "rc", scope: !951, file: !3, line: 148, type: !116)
!955 = !DILocation(line: 150, column: 5, scope: !956)
!956 = distinct !DILexicalBlock(scope: !951, file: !3, line: 150, column: 5)
!957 = !DILocation(line: 150, column: 14, scope: !956)
!958 = !DILocation(line: 150, column: 5, scope: !951)
!959 = !DILocation(line: 151, column: 8, scope: !960)
!960 = distinct !DILexicalBlock(scope: !956, file: !3, line: 150, column: 19)
!961 = !DILocation(line: 148, column: 6, scope: !951)
!962 = !DILocation(line: 152, column: 9, scope: !963)
!963 = distinct !DILexicalBlock(scope: !960, file: !3, line: 152, column: 6)
!964 = !DILocation(line: 152, column: 6, scope: !960)
!965 = !DILocation(line: 153, column: 19, scope: !966)
!966 = distinct !DILexicalBlock(scope: !963, file: !3, line: 152, column: 15)
!967 = !DILocation(line: 153, column: 10, scope: !966)
!968 = !DILocation(line: 147, column: 8, scope: !951)
!969 = !DILocation(line: 154, column: 4, scope: !966)
!970 = !DILocation(line: 155, column: 4, scope: !966)
!971 = !DILocation(line: 157, column: 8, scope: !960)
!972 = !DILocation(line: 158, column: 9, scope: !973)
!973 = distinct !DILexicalBlock(scope: !960, file: !3, line: 158, column: 6)
!974 = !DILocation(line: 158, column: 6, scope: !960)
!975 = !DILocation(line: 159, column: 19, scope: !976)
!976 = distinct !DILexicalBlock(scope: !973, file: !3, line: 158, column: 15)
!977 = !DILocation(line: 159, column: 10, scope: !976)
!978 = !DILocation(line: 160, column: 4, scope: !976)
!979 = !DILocation(line: 161, column: 4, scope: !976)
!980 = !DILocation(line: 166, column: 1, scope: !951)
!981 = distinct !DISubprogram(name: "mosquitto__daemonise", scope: !3, file: !3, line: 169, type: !982, isLocal: false, isDefinition: true, scopeLine: 170, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !984)
!982 = !DISubroutineType(types: !983)
!983 = !{null}
!984 = !{!985, !986}
!985 = !DILocalVariable(name: "err", scope: !981, file: !3, line: 172, type: !124)
!986 = !DILocalVariable(name: "pid", scope: !981, file: !3, line: 173, type: !987)
!987 = !DIDerivedType(tag: DW_TAG_typedef, name: "pid_t", file: !988, line: 263, baseType: !989)
!988 = !DIFile(filename: "/usr/include/unistd.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!989 = !DIDerivedType(tag: DW_TAG_typedef, name: "__pid_t", file: !133, line: 142, baseType: !116)
!990 = !DILocation(line: 175, column: 8, scope: !981)
!991 = !DILocation(line: 173, column: 8, scope: !981)
!992 = !DILocation(line: 176, column: 9, scope: !993)
!993 = distinct !DILexicalBlock(scope: !981, file: !3, line: 176, column: 5)
!994 = !DILocation(line: 176, column: 5, scope: !981)
!995 = !DILocation(line: 177, column: 18, scope: !996)
!996 = distinct !DILexicalBlock(scope: !993, file: !3, line: 176, column: 13)
!997 = !DILocation(line: 177, column: 9, scope: !996)
!998 = !DILocation(line: 172, column: 8, scope: !981)
!999 = !DILocation(line: 178, column: 3, scope: !996)
!1000 = !DILocation(line: 179, column: 3, scope: !996)
!1001 = !DILocation(line: 181, column: 9, scope: !1002)
!1002 = distinct !DILexicalBlock(scope: !981, file: !3, line: 181, column: 5)
!1003 = !DILocation(line: 181, column: 5, scope: !981)
!1004 = !DILocation(line: 182, column: 3, scope: !1005)
!1005 = distinct !DILexicalBlock(scope: !1002, file: !3, line: 181, column: 13)
!1006 = !DILocation(line: 184, column: 5, scope: !1007)
!1007 = distinct !DILexicalBlock(scope: !981, file: !3, line: 184, column: 5)
!1008 = !DILocation(line: 184, column: 14, scope: !1007)
!1009 = !DILocation(line: 184, column: 5, scope: !981)
!1010 = !DILocation(line: 185, column: 18, scope: !1011)
!1011 = distinct !DILexicalBlock(scope: !1007, file: !3, line: 184, column: 18)
!1012 = !DILocation(line: 185, column: 9, scope: !1011)
!1013 = !DILocation(line: 186, column: 3, scope: !1011)
!1014 = !DILocation(line: 187, column: 3, scope: !1011)
!1015 = !DILocation(line: 190, column: 2, scope: !1016)
!1016 = distinct !DILexicalBlock(scope: !1017, file: !3, line: 190, column: 2)
!1017 = distinct !DILexicalBlock(scope: !981, file: !3, line: 190, column: 2)
!1018 = !{!876, !876, i64 0}
!1019 = !DILocation(line: 190, column: 2, scope: !1017)
!1020 = !DILocation(line: 191, column: 2, scope: !1021)
!1021 = distinct !DILexicalBlock(scope: !1022, file: !3, line: 191, column: 2)
!1022 = distinct !DILexicalBlock(scope: !981, file: !3, line: 191, column: 2)
!1023 = !DILocation(line: 191, column: 2, scope: !1022)
!1024 = !DILocation(line: 192, column: 2, scope: !1025)
!1025 = distinct !DILexicalBlock(scope: !1026, file: !3, line: 192, column: 2)
!1026 = distinct !DILexicalBlock(scope: !981, file: !3, line: 192, column: 2)
!1027 = !DILocation(line: 192, column: 2, scope: !1026)
!1028 = !DILocation(line: 196, column: 1, scope: !981)
!1029 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 199, type: !1030, isLocal: false, isDefinition: true, scopeLine: 200, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !1032)
!1030 = !DISubroutineType(types: !1031)
!1031 = !{!116, !116, !588}
!1032 = !{!1033, !1034, !1035, !1036, !1037, !1038, !1039, !1040, !1041, !1042, !1043, !1050, !1051}
!1033 = !DILocalVariable(name: "argc", arg: 1, scope: !1029, file: !3, line: 199, type: !116)
!1034 = !DILocalVariable(name: "argv", arg: 2, scope: !1029, file: !3, line: 199, type: !588)
!1035 = !DILocalVariable(name: "listensock", scope: !1029, file: !3, line: 201, type: !346)
!1036 = !DILocalVariable(name: "listensock_count", scope: !1029, file: !3, line: 202, type: !116)
!1037 = !DILocalVariable(name: "listensock_index", scope: !1029, file: !3, line: 203, type: !116)
!1038 = !DILocalVariable(name: "config", scope: !1029, file: !3, line: 204, type: !717)
!1039 = !DILocalVariable(name: "i", scope: !1029, file: !3, line: 205, type: !116)
!1040 = !DILocalVariable(name: "j", scope: !1029, file: !3, line: 205, type: !116)
!1041 = !DILocalVariable(name: "pid", scope: !1029, file: !3, line: 206, type: !736)
!1042 = !DILocalVariable(name: "rc", scope: !1029, file: !3, line: 207, type: !116)
!1043 = !DILocalVariable(name: "tv", scope: !1029, file: !3, line: 211, type: !1044)
!1044 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "timeval", file: !1045, line: 8, size: 128, elements: !1046)
!1045 = !DIFile(filename: "/usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h", directory: "/home/ryan/git/TARGETS/mosquitto/src")
!1046 = !{!1047, !1048}
!1047 = !DIDerivedType(tag: DW_TAG_member, name: "tv_sec", scope: !1044, file: !1045, line: 10, baseType: !140, size: 64)
!1048 = !DIDerivedType(tag: DW_TAG_member, name: "tv_usec", scope: !1044, file: !1045, line: 11, baseType: !1049, size: 64, offset: 64)
!1049 = !DIDerivedType(tag: DW_TAG_typedef, name: "__suseconds_t", file: !133, line: 150, baseType: !141)
!1050 = !DILocalVariable(name: "ctxt", scope: !1029, file: !3, line: 213, type: !117)
!1051 = !DILocalVariable(name: "ctxt_tmp", scope: !1029, file: !3, line: 213, type: !117)
!1052 = !DILocation(line: 199, column: 14, scope: !1029)
!1053 = !DILocation(line: 199, column: 26, scope: !1029)
!1054 = !DILocation(line: 201, column: 15, scope: !1029)
!1055 = !DILocation(line: 202, column: 6, scope: !1029)
!1056 = !DILocation(line: 203, column: 6, scope: !1029)
!1057 = !DILocation(line: 204, column: 2, scope: !1029)
!1058 = !DILocation(line: 211, column: 2, scope: !1029)
!1059 = !DILocation(line: 211, column: 17, scope: !1029)
!1060 = !DILocation(line: 235, column: 2, scope: !1029)
!1061 = !DILocation(line: 236, column: 11, scope: !1029)
!1062 = !{!1063, !880, i64 0}
!1063 = !{!"timeval", !880, i64 0, !880, i64 8}
!1064 = !DILocation(line: 236, column: 23, scope: !1029)
!1065 = !{!1063, !880, i64 8}
!1066 = !DILocation(line: 236, column: 18, scope: !1029)
!1067 = !DILocation(line: 236, column: 8, scope: !1029)
!1068 = !DILocation(line: 236, column: 2, scope: !1029)
!1069 = !DILocation(line: 243, column: 2, scope: !1029)
!1070 = !DILocation(line: 245, column: 2, scope: !1029)
!1071 = !DILocation(line: 204, column: 27, scope: !1029)
!1072 = !DILocation(line: 247, column: 2, scope: !1029)
!1073 = !DILocation(line: 248, column: 7, scope: !1029)
!1074 = !DILocation(line: 207, column: 6, scope: !1029)
!1075 = !DILocation(line: 249, column: 8, scope: !1076)
!1076 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 249, column: 5)
!1077 = !DILocation(line: 249, column: 5, scope: !1029)
!1078 = !DILocation(line: 250, column: 16, scope: !1029)
!1079 = !{!1080, !876, i64 112}
!1080 = !{!"mosquitto_db", !880, i64 0, !876, i64 8, !876, i64 16, !876, i64 24, !876, i64 32, !876, i64 40, !876, i64 48, !876, i64 56, !876, i64 64, !876, i64 72, !876, i64 80, !875, i64 88, !875, i64 92, !880, i64 96, !876, i64 104, !876, i64 112, !875, i64 120, !872, i64 124, !875, i64 128, !875, i64 132, !875, i64 136, !875, i64 140, !876, i64 144, !875, i64 152}
!1081 = !DILocation(line: 252, column: 12, scope: !1082)
!1082 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 252, column: 5)
!1083 = !{!871, !872, i64 25}
!1084 = !{i8 0, i8 2}
!1085 = !DILocation(line: 252, column: 5, scope: !1029)
!1086 = !DILocation(line: 253, column: 3, scope: !1087)
!1087 = distinct !DILexicalBlock(scope: !1082, file: !3, line: 252, column: 19)
!1088 = !DILocation(line: 256, column: 12, scope: !1089)
!1089 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 256, column: 5)
!1090 = !DILocation(line: 256, column: 19, scope: !1089)
!1091 = !DILocation(line: 256, column: 29, scope: !1089)
!1092 = !{!871, !876, i64 400}
!1093 = !DILocation(line: 256, column: 22, scope: !1089)
!1094 = !DILocation(line: 256, column: 5, scope: !1029)
!1095 = !DILocation(line: 257, column: 9, scope: !1096)
!1096 = distinct !DILexicalBlock(scope: !1089, file: !3, line: 256, column: 38)
!1097 = !DILocation(line: 206, column: 8, scope: !1029)
!1098 = !DILocation(line: 258, column: 6, scope: !1099)
!1099 = distinct !DILexicalBlock(scope: !1096, file: !3, line: 258, column: 6)
!1100 = !DILocation(line: 258, column: 6, scope: !1096)
!1101 = !DILocation(line: 259, column: 23, scope: !1102)
!1102 = distinct !DILexicalBlock(scope: !1099, file: !3, line: 258, column: 10)
!1103 = !DILocation(line: 259, column: 4, scope: !1102)
!1104 = !DILocation(line: 260, column: 4, scope: !1102)
!1105 = !DILocation(line: 265, column: 2, scope: !1096)
!1106 = !DILocation(line: 262, column: 4, scope: !1107)
!1107 = distinct !DILexicalBlock(scope: !1099, file: !3, line: 261, column: 8)
!1108 = !DILocation(line: 263, column: 4, scope: !1107)
!1109 = !DILocation(line: 267, column: 7, scope: !1029)
!1110 = !DILocation(line: 268, column: 8, scope: !1111)
!1111 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 268, column: 5)
!1112 = !DILocation(line: 268, column: 5, scope: !1029)
!1113 = !DILocation(line: 269, column: 3, scope: !1114)
!1114 = distinct !DILexicalBlock(scope: !1111, file: !3, line: 268, column: 28)
!1115 = !DILocation(line: 270, column: 3, scope: !1114)
!1116 = !DILocation(line: 275, column: 5, scope: !1117)
!1117 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 275, column: 5)
!1118 = !DILocation(line: 275, column: 5, scope: !1029)
!1119 = !DILocation(line: 279, column: 2, scope: !1029)
!1120 = !DILocation(line: 280, column: 12, scope: !1121)
!1121 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 280, column: 5)
!1122 = !{!1080, !876, i64 104}
!1123 = !DILocation(line: 280, column: 5, scope: !1121)
!1124 = !DILocation(line: 280, column: 5, scope: !1029)
!1125 = !DILocation(line: 281, column: 3, scope: !1126)
!1126 = distinct !DILexicalBlock(scope: !1121, file: !3, line: 280, column: 24)
!1127 = !DILocation(line: 282, column: 2, scope: !1126)
!1128 = !DILocation(line: 283, column: 3, scope: !1129)
!1129 = distinct !DILexicalBlock(scope: !1121, file: !3, line: 282, column: 7)
!1130 = !DILocation(line: 286, column: 7, scope: !1029)
!1131 = !DILocation(line: 287, column: 5, scope: !1132)
!1132 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 287, column: 5)
!1133 = !DILocation(line: 287, column: 5, scope: !1029)
!1134 = !DILocation(line: 288, column: 7, scope: !1029)
!1135 = !DILocation(line: 289, column: 5, scope: !1136)
!1136 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 289, column: 5)
!1137 = !DILocation(line: 289, column: 5, scope: !1029)
!1138 = !DILocation(line: 292, column: 2, scope: !1029)
!1139 = !DILocation(line: 205, column: 6, scope: !1029)
!1140 = !DILocation(line: 296, column: 20, scope: !1141)
!1141 = distinct !DILexicalBlock(scope: !1142, file: !3, line: 296, column: 2)
!1142 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 296, column: 2)
!1143 = !{!871, !875, i64 304}
!1144 = !DILocation(line: 296, column: 12, scope: !1141)
!1145 = !DILocation(line: 296, column: 2, scope: !1142)
!1146 = !DILocation(line: 297, column: 13, scope: !1147)
!1147 = distinct !DILexicalBlock(scope: !1148, file: !3, line: 297, column: 6)
!1148 = distinct !DILexicalBlock(scope: !1141, file: !3, line: 296, column: 40)
!1149 = !{!871, !876, i64 296}
!1150 = !DILocation(line: 297, column: 26, scope: !1147)
!1151 = !{!877, !873, i64 56}
!1152 = !DILocation(line: 297, column: 35, scope: !1147)
!1153 = !DILocation(line: 297, column: 6, scope: !1148)
!1154 = !DILocation(line: 297, column: 6, scope: !1147)
!1155 = !DILocation(line: 298, column: 7, scope: !1156)
!1156 = distinct !DILexicalBlock(scope: !1157, file: !3, line: 298, column: 7)
!1157 = distinct !DILexicalBlock(scope: !1147, file: !3, line: 297, column: 46)
!1158 = !DILocation(line: 298, column: 7, scope: !1157)
!1159 = !DILocation(line: 299, column: 5, scope: !1160)
!1160 = distinct !DILexicalBlock(scope: !1156, file: !3, line: 298, column: 48)
!1161 = !DILocation(line: 300, column: 15, scope: !1162)
!1162 = distinct !DILexicalBlock(scope: !1160, file: !3, line: 300, column: 8)
!1163 = !DILocation(line: 300, column: 8, scope: !1162)
!1164 = !DILocation(line: 300, column: 8, scope: !1160)
!1165 = !DILocation(line: 301, column: 6, scope: !1166)
!1166 = distinct !DILexicalBlock(scope: !1162, file: !3, line: 300, column: 24)
!1167 = !DILocation(line: 302, column: 5, scope: !1166)
!1168 = !DILocation(line: 305, column: 31, scope: !1157)
!1169 = !DILocation(line: 305, column: 44, scope: !1157)
!1170 = !{!877, !875, i64 48}
!1171 = !DILocation(line: 305, column: 21, scope: !1157)
!1172 = !DILocation(line: 306, column: 68, scope: !1157)
!1173 = !DILocation(line: 306, column: 67, scope: !1157)
!1174 = !DILocation(line: 306, column: 17, scope: !1157)
!1175 = !DILocation(line: 307, column: 8, scope: !1176)
!1176 = distinct !DILexicalBlock(scope: !1157, file: !3, line: 307, column: 7)
!1177 = !DILocation(line: 307, column: 7, scope: !1157)
!1178 = !DILocation(line: 308, column: 5, scope: !1179)
!1179 = distinct !DILexicalBlock(scope: !1176, file: !3, line: 307, column: 19)
!1180 = !DILocation(line: 309, column: 15, scope: !1181)
!1181 = distinct !DILexicalBlock(scope: !1179, file: !3, line: 309, column: 8)
!1182 = !DILocation(line: 309, column: 8, scope: !1181)
!1183 = !DILocation(line: 309, column: 8, scope: !1179)
!1184 = !DILocation(line: 310, column: 6, scope: !1185)
!1185 = distinct !DILexicalBlock(scope: !1181, file: !3, line: 309, column: 24)
!1186 = !DILocation(line: 311, column: 5, scope: !1185)
!1187 = !DILocation(line: 205, column: 9, scope: !1029)
!1188 = !DILocation(line: 314, column: 22, scope: !1189)
!1189 = distinct !DILexicalBlock(scope: !1190, file: !3, line: 314, column: 4)
!1190 = distinct !DILexicalBlock(scope: !1157, file: !3, line: 314, column: 4)
!1191 = !DILocation(line: 314, column: 35, scope: !1189)
!1192 = !DILocation(line: 314, column: 14, scope: !1189)
!1193 = !DILocation(line: 314, column: 4, scope: !1190)
!1194 = !DILocation(line: 315, column: 28, scope: !1195)
!1195 = distinct !DILexicalBlock(scope: !1196, file: !3, line: 315, column: 8)
!1196 = distinct !DILexicalBlock(scope: !1189, file: !3, line: 314, column: 51)
!1197 = !{!877, !876, i64 40}
!1198 = !DILocation(line: 315, column: 8, scope: !1195)
!1199 = !DILocation(line: 315, column: 37, scope: !1195)
!1200 = !DILocation(line: 315, column: 8, scope: !1196)
!1201 = !DILocation(line: 316, column: 6, scope: !1202)
!1202 = distinct !DILexicalBlock(scope: !1195, file: !3, line: 315, column: 55)
!1203 = !DILocation(line: 317, column: 16, scope: !1204)
!1204 = distinct !DILexicalBlock(scope: !1202, file: !3, line: 317, column: 9)
!1205 = !DILocation(line: 317, column: 9, scope: !1204)
!1206 = !DILocation(line: 317, column: 9, scope: !1202)
!1207 = !DILocation(line: 318, column: 7, scope: !1208)
!1208 = distinct !DILexicalBlock(scope: !1204, file: !3, line: 317, column: 25)
!1209 = !DILocation(line: 319, column: 6, scope: !1208)
!1210 = !DILocation(line: 322, column: 5, scope: !1196)
!1211 = !DILocation(line: 322, column: 34, scope: !1196)
!1212 = !DILocation(line: 323, column: 21, scope: !1196)
!1213 = !DILocation(line: 314, column: 48, scope: !1189)
!1214 = distinct !{!1214, !1193, !1215}
!1215 = !DILocation(line: 324, column: 4, scope: !1190)
!1216 = !DILocation(line: 296, column: 37, scope: !1141)
!1217 = distinct !{!1217, !1145, !1218}
!1218 = !DILocation(line: 334, column: 2, scope: !1142)
!1219 = !DILocation(line: 336, column: 7, scope: !1029)
!1220 = !DILocation(line: 337, column: 8, scope: !1221)
!1221 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 337, column: 5)
!1222 = !DILocation(line: 337, column: 5, scope: !1029)
!1223 = !DILocation(line: 339, column: 2, scope: !1029)
!1224 = !DILocation(line: 340, column: 2, scope: !1029)
!1225 = !DILocation(line: 342, column: 2, scope: !1029)
!1226 = !DILocation(line: 345, column: 2, scope: !1029)
!1227 = !DILocation(line: 346, column: 2, scope: !1029)
!1228 = !DILocation(line: 347, column: 2, scope: !1029)
!1229 = !DILocation(line: 354, column: 20, scope: !1230)
!1230 = distinct !DILexicalBlock(scope: !1231, file: !3, line: 354, column: 2)
!1231 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 354, column: 2)
!1232 = !{!871, !875, i64 440}
!1233 = !DILocation(line: 354, column: 12, scope: !1230)
!1234 = !DILocation(line: 354, column: 2, scope: !1231)
!1235 = !DILocation(line: 355, column: 36, scope: !1236)
!1236 = distinct !DILexicalBlock(scope: !1237, file: !3, line: 355, column: 6)
!1237 = distinct !DILexicalBlock(scope: !1230, file: !3, line: 354, column: 38)
!1238 = !{!871, !876, i64 432}
!1239 = !DILocation(line: 355, column: 29, scope: !1236)
!1240 = !DILocation(line: 355, column: 6, scope: !1236)
!1241 = !DILocation(line: 355, column: 6, scope: !1237)
!1242 = !DILocation(line: 357, column: 13, scope: !1243)
!1243 = distinct !DILexicalBlock(scope: !1236, file: !3, line: 355, column: 49)
!1244 = !DILocation(line: 357, column: 24, scope: !1243)
!1245 = !{!1246, !876, i64 0}
!1246 = !{!"mosquitto__bridge", !876, i64 0, !876, i64 8, !875, i64 16, !875, i64 20, !880, i64 24, !875, i64 32, !872, i64 36, !872, i64 37, !872, i64 38, !872, i64 39, !875, i64 40, !876, i64 48, !875, i64 56, !872, i64 60, !873, i64 64, !880, i64 72, !876, i64 80, !876, i64 88, !876, i64 96, !876, i64 104, !876, i64 112, !876, i64 120, !876, i64 128, !872, i64 136, !872, i64 137, !873, i64 140, !875, i64 144, !875, i64 148, !875, i64 152, !875, i64 156, !875, i64 160, !872, i64 164, !872, i64 165, !872, i64 166, !872, i64 167, !872, i64 168, !876, i64 176, !876, i64 184, !876, i64 192, !876, i64 200, !876, i64 208, !876, i64 216, !876, i64 224, !876, i64 232}
!1247 = !DILocation(line: 356, column: 4, scope: !1243)
!1248 = !DILocation(line: 358, column: 3, scope: !1243)
!1249 = !DILocation(line: 354, column: 35, scope: !1230)
!1250 = distinct !{!1250, !1234, !1251}
!1251 = !DILocation(line: 359, column: 2, scope: !1231)
!1252 = !DILocation(line: 366, column: 6, scope: !1029)
!1253 = !DILocation(line: 367, column: 7, scope: !1029)
!1254 = !DILocation(line: 369, column: 2, scope: !1029)
!1255 = !DILocation(line: 383, column: 2, scope: !1256)
!1256 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 383, column: 2)
!1257 = !{!1080, !876, i64 32}
!1258 = !DILocation(line: 213, column: 20, scope: !1029)
!1259 = !{!1260, !876, i64 512}
!1260 = !{!"mosquitto", !875, i64 0, !873, i64 4, !876, i64 8, !876, i64 16, !876, i64 24, !876, i64 32, !878, i64 40, !878, i64 42, !873, i64 44, !880, i64 48, !880, i64 56, !880, i64 64, !1261, i64 72, !876, i64 112, !876, i64 120, !876, i64 128, !876, i64 136, !876, i64 144, !875, i64 152, !875, i64 156, !875, i64 160, !880, i64 168, !876, i64 176, !876, i64 184, !876, i64 192, !876, i64 200, !876, i64 208, !876, i64 216, !876, i64 224, !876, i64 232, !876, i64 240, !876, i64 248, !876, i64 256, !875, i64 264, !872, i64 268, !872, i64 269, !872, i64 270, !876, i64 272, !876, i64 280, !873, i64 288, !876, i64 296, !872, i64 304, !872, i64 305, !872, i64 306, !875, i64 308, !880, i64 312, !872, i64 320, !872, i64 321, !872, i64 322, !876, i64 328, !1262, i64 336, !1262, i64 384, !876, i64 432, !876, i64 440, !876, i64 448, !876, i64 456, !876, i64 464, !876, i64 472, !875, i64 480, !875, i64 484, !875, i64 488, !872, i64 492, !872, i64 493, !873, i64 494, !1263, i64 496, !1263, i64 552, !876, i64 608, !876, i64 616, !875, i64 624}
!1261 = !{!"mosquitto__packet", !876, i64 0, !876, i64 8, !875, i64 16, !875, i64 20, !875, i64 24, !875, i64 28, !875, i64 32, !878, i64 36, !873, i64 38, !873, i64 39}
!1262 = !{!"mosquitto_msg_data", !876, i64 0, !876, i64 8, !880, i64 16, !880, i64 24, !875, i64 32, !875, i64 36, !875, i64 40, !878, i64 44}
!1263 = !{!"UT_hash_handle", !876, i64 0, !876, i64 8, !876, i64 16, !876, i64 24, !876, i64 32, !876, i64 40, !875, i64 48, !875, i64 52}
!1264 = !DILocation(line: 213, column: 27, scope: !1029)
!1265 = !DILocation(line: 384, column: 3, scope: !1266)
!1266 = distinct !DILexicalBlock(scope: !1267, file: !3, line: 383, column: 57)
!1267 = distinct !DILexicalBlock(scope: !1256, file: !3, line: 383, column: 2)
!1268 = !DILocation(line: 383, column: 2, scope: !1267)
!1269 = !DILocation(line: 386, column: 2, scope: !1029)
!1270 = !DILocation(line: 389, column: 12, scope: !1271)
!1271 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 389, column: 5)
!1272 = !{!871, !872, i64 364}
!1273 = !DILocation(line: 389, column: 5, scope: !1029)
!1274 = !DILocation(line: 390, column: 3, scope: !1275)
!1275 = distinct !DILexicalBlock(scope: !1271, file: !3, line: 389, column: 24)
!1276 = !DILocation(line: 391, column: 2, scope: !1275)
!1277 = !DILocation(line: 393, column: 2, scope: !1029)
!1278 = !DILocation(line: 395, column: 2, scope: !1279)
!1279 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 395, column: 2)
!1280 = !DILocation(line: 401, column: 3, scope: !1281)
!1281 = distinct !DILexicalBlock(scope: !1282, file: !3, line: 395, column: 57)
!1282 = distinct !DILexicalBlock(scope: !1279, file: !3, line: 395, column: 2)
!1283 = !DILocation(line: 395, column: 2, scope: !1282)
!1284 = !DILocation(line: 404, column: 2, scope: !1285)
!1285 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 404, column: 2)
!1286 = !{!1080, !876, i64 40}
!1287 = !{!1260, !876, i64 568}
!1288 = !DILocation(line: 405, column: 3, scope: !1289)
!1289 = distinct !DILexicalBlock(scope: !1290, file: !3, line: 404, column: 61)
!1290 = distinct !DILexicalBlock(scope: !1285, file: !3, line: 404, column: 2)
!1291 = !DILocation(line: 404, column: 2, scope: !1290)
!1292 = !DILocation(line: 408, column: 20, scope: !1293)
!1293 = distinct !DILexicalBlock(scope: !1294, file: !3, line: 408, column: 2)
!1294 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 408, column: 2)
!1295 = !{!1080, !875, i64 88}
!1296 = !DILocation(line: 408, column: 12, scope: !1293)
!1297 = !{!1080, !876, i64 56}
!1298 = !DILocation(line: 408, column: 2, scope: !1294)
!1299 = !DILocation(line: 409, column: 6, scope: !1300)
!1300 = distinct !DILexicalBlock(scope: !1301, file: !3, line: 409, column: 6)
!1301 = distinct !DILexicalBlock(scope: !1293, file: !3, line: 408, column: 38)
!1302 = !DILocation(line: 409, column: 6, scope: !1301)
!1303 = !DILocation(line: 410, column: 4, scope: !1304)
!1304 = distinct !DILexicalBlock(scope: !1300, file: !3, line: 409, column: 24)
!1305 = !DILocation(line: 411, column: 3, scope: !1304)
!1306 = !DILocation(line: 408, column: 35, scope: !1293)
!1307 = distinct !{!1307, !1298, !1308}
!1308 = !DILocation(line: 412, column: 2, scope: !1294)
!1309 = !DILocation(line: 413, column: 18, scope: !1029)
!1310 = !DILocation(line: 413, column: 2, scope: !1029)
!1311 = !DILocation(line: 415, column: 2, scope: !1029)
!1312 = !DILocation(line: 417, column: 2, scope: !1029)
!1313 = !DILocation(line: 419, column: 5, scope: !1314)
!1314 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 419, column: 5)
!1315 = !DILocation(line: 419, column: 5, scope: !1029)
!1316 = !DILocation(line: 420, column: 13, scope: !1317)
!1317 = distinct !DILexicalBlock(scope: !1318, file: !3, line: 420, column: 3)
!1318 = distinct !DILexicalBlock(scope: !1319, file: !3, line: 420, column: 3)
!1319 = distinct !DILexicalBlock(scope: !1314, file: !3, line: 419, column: 16)
!1320 = !DILocation(line: 420, column: 3, scope: !1318)
!1321 = !DILocation(line: 421, column: 7, scope: !1322)
!1322 = distinct !DILexicalBlock(scope: !1323, file: !3, line: 421, column: 7)
!1323 = distinct !DILexicalBlock(scope: !1317, file: !3, line: 420, column: 36)
!1324 = !DILocation(line: 421, column: 21, scope: !1322)
!1325 = !DILocation(line: 421, column: 7, scope: !1323)
!1326 = !DILocation(line: 423, column: 5, scope: !1327)
!1327 = distinct !DILexicalBlock(scope: !1322, file: !3, line: 421, column: 39)
!1328 = !DILocation(line: 427, column: 4, scope: !1327)
!1329 = !DILocation(line: 420, column: 33, scope: !1317)
!1330 = distinct !{!1330, !1320, !1331}
!1331 = !DILocation(line: 428, column: 3, scope: !1318)
!1332 = !DILocation(line: 429, column: 3, scope: !1319)
!1333 = !DILocation(line: 430, column: 2, scope: !1319)
!1334 = !DILocation(line: 432, column: 2, scope: !1029)
!1335 = !DILocation(line: 434, column: 12, scope: !1336)
!1336 = distinct !DILexicalBlock(scope: !1029, file: !3, line: 434, column: 5)
!1337 = !DILocation(line: 434, column: 5, scope: !1336)
!1338 = !DILocation(line: 434, column: 5, scope: !1029)
!1339 = !DILocation(line: 435, column: 3, scope: !1340)
!1340 = distinct !DILexicalBlock(scope: !1336, file: !3, line: 434, column: 21)
!1341 = !DILocation(line: 436, column: 2, scope: !1340)
!1342 = !DILocation(line: 438, column: 2, scope: !1029)
!1343 = !DILocation(line: 439, column: 25, scope: !1029)
!1344 = !DILocation(line: 439, column: 2, scope: !1029)
!1345 = !DILocation(line: 440, column: 2, scope: !1029)
!1346 = !DILocation(line: 442, column: 2, scope: !1029)
!1347 = !DILocation(line: 443, column: 1, scope: !1029)
!1348 = distinct !DISubprogram(name: "memset", scope: !820, file: !820, line: 12, type: !1349, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagPrototyped, isOptimized: false, unit: !819, variables: !821)
!1349 = !DISubroutineType(types: !1350)
!1350 = !{!191, !191, !116, !1351}
!1351 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !1352, line: 62, baseType: !332)
!1352 = !DIFile(filename: "/usr/lib/llvm-6.0/lib/clang/6.0.0/include/stddef.h", directory: "/home/ryan/git/klee/build/runtime/FreeStanding")
!1353 = !DILocalVariable(name: "dst", arg: 1, scope: !1348, file: !820, line: 12, type: !191)
!1354 = !DILocation(line: 12, column: 20, scope: !1348)
!1355 = !DILocalVariable(name: "s", arg: 2, scope: !1348, file: !820, line: 12, type: !116)
!1356 = !DILocation(line: 12, column: 29, scope: !1348)
!1357 = !DILocalVariable(name: "count", arg: 3, scope: !1348, file: !820, line: 12, type: !1351)
!1358 = !DILocation(line: 12, column: 39, scope: !1348)
!1359 = !DILocalVariable(name: "a", scope: !1348, file: !820, line: 13, type: !124)
!1360 = !DILocation(line: 13, column: 9, scope: !1348)
!1361 = !DILocation(line: 13, column: 13, scope: !1348)
!1362 = !DILocation(line: 14, column: 3, scope: !1348)
!1363 = !DILocation(line: 14, column: 15, scope: !1348)
!1364 = !DILocation(line: 14, column: 18, scope: !1348)
!1365 = !DILocation(line: 15, column: 12, scope: !1348)
!1366 = !DILocation(line: 15, column: 7, scope: !1348)
!1367 = !DILocation(line: 15, column: 10, scope: !1348)
!1368 = distinct !{!1368, !1362, !1365}
!1369 = !DILocation(line: 16, column: 10, scope: !1348)
!1370 = !DILocation(line: 16, column: 3, scope: !1348)
