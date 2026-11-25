// Benstore (iOS) — engine: account, signing identity, pairing file, installed apps, install/refresh.
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface BSApp : NSObject
@property NSString *bundleID, *originalBundleID, *name, *version, *cacheDir, *iconFile;
@property (nullable) NSString *appleID;      // which Apple ID installed it (nil = the main one)
@property NSDate *expiry, *signedAt;
@property BOOL isBenstore;
- (NSString *)expiryText;
- (UIColor *)expiryColor;
- (double)lifeFraction;      // 0..1 of the 7-day window left
- (NSString *)ringText;
@end

@protocol BSStoreUI <NSObject>
- (void)storeProgress:(float)p status:(NSString *)status;
- (void)storeFinished:(nullable NSString *)error message:(nullable NSString *)msg;
- (nullable NSString *)storeAskCode:(NSString *)prompt;     // called on a background thread, blocks
- (void)storeGuide:(nullable NSString *)title text:(nullable NSString *)text;   // show (or hide, title nil) a step-by-step guide with Cancel
- (NSInteger)storeChooseApp:(NSString *)title message:(NSString *)message options:(NSArray<NSString *> *)names;   // background thread, blocks; -1 = cancel
- (void)storeWaitingForVPN:(BOOL)waiting;                  // show / hide the "Connect LocalDevVPN" sheet
- (void)storeLog:(NSString *)line;
@end

@interface BSStore : NSObject
+ (instancetype)shared;
@property (weak) id<BSStoreUI> ui;
@property (readonly) NSString *docs;          // private state dir (Library/Application Support/Benstore)
@property (readonly) BOOL vpnConnected;        // last known LocalDevVPN state
@property (readonly) BOOL onWiFi;
@property BOOL cellularInstalls;              // Experimental: on cellular, briefly turn cellular data off for the install step
@property NSString *cellOffShortcut, *cellOnShortcut;
@property BOOL selfInstalling;                // Benstore is installing over itself (it will be closed)
- (void)checkCellularDataPending;   // optional Shortcut names that turn cellular data off / on              // installs need Wi-Fi: iOS blocks the VPN loop on cellular data
@property BOOL cancelVPNWait;
@property BOOL headless;                      // background task: never prompt the user
@property (readonly, nullable) NSString *appleID, *teamID, *teamName, *udid;
@property (readonly) BOOL hasPairingFile, hasIdentity, signedIn, busy;
@property NSString *anisetteServers;
@property NSString *deviceAddress;
@property (readonly) NSArray<BSApp *> *apps;
@property (readonly) NSArray<NSString *> *otherAppsOnMainID;
@property (readonly) NSArray<NSDictionary *> *otherAppsOnMain;   // {id, name} for the same apps
@property BOOL quiet;                          // background scan: no status bar, no prompts
- (void)scanDeviceApps;
- (void)removeOtherApp:(NSDictionary *)app;
// ---- Park: save an app's data + keep its signed copy, then remove it to free a slot. Unpark brings both back. ----
@property (readonly) NSArray<NSDictionary *> *parkedApps;   // BSApp dicts + dataBytes, parkedAt
@property BOOL preferBackup;   // "Use Backup Apple ID": new installs sign with the first backup; main is the fallback
// Extra memory capabilities per app (SL_CAP_IML = 1 Increased Memory Limit, SL_CAP_EVA = 2 Extended Virtual Addressing)
- (int)capsFor:(BSApp *)app;
- (void)setCaps:(int)caps forApp:(BSApp *)app;   // saves and re-signs the app
- (UIImage *)customIconFor:(BSApp *)app;            // nil = the app's own icon
- (void)setCustomIcon:(UIImage *)img forApp:(BSApp *)app;   // nil resets; saves and re-signs the app
- (void)deleteAppID:(NSString *)appIdId for:(NSString *)email done:(void (^)(NSString *error))done;   // email nil = main Apple ID
- (void)parkApp:(BSApp *)app;
- (void)unparkApp:(NSDictionary *)parked;
- (void)deleteParked:(NSDictionary *)parked;
+ (BSApp *)appFromParked:(NSDictionary *)parked;   // names of apps signed by the main Apple ID that Benstore didn't install     // includes Benstore itself first
@property (readonly) NSMutableArray<NSString *> *logLines;

- (void)signInWithAppleID:(NSString *)email password:(NSString *)password;
- (void)signOut;
// ---- backup Apple IDs: used when the main one hits a free-account limit (3 apps / 10 App IDs a week) ----
@property (readonly) NSArray<NSString *> *backupAppleIDs;
- (void)addBackupAppleID:(NSString *)email password:(NSString *)password;
- (void)removeBackupAppleID:(NSString *)email;
// App ID tracker: rows of {name, identifier, expires (NSDate or NSNull)} for the main (nil) or a backup Apple ID
- (void)fetchAppIDsFor:(nullable NSString *)email done:(void (^)(NSArray<NSDictionary *> *_Nullable ids, NSString *_Nullable err))done;
- (BOOL)importPairingFile:(NSURL *)url error:(NSString *_Nullable *_Nullable)err;
- (void)installIPA:(NSURL *)url;
- (void)refreshApp:(BSApp *)app;
- (void)refreshAll;
- (void)removeApp:(BSApp *)app;
- (void)testConnection;
- (void)reload;
- (void)checkVPN:(void (^)(BOOL connected))done;
- (void)refreshAllCompletion:(nullable void (^)(BOOL ok))done;
- (void)scheduleReminders;
// ---- self-update: reads a small JSON file you host (default: benjaminsoble91-eng.github.io/benstore) ----
@property NSString *updateURL;
// done(info, err): info is nil when already up to date. info keys: version, build, notes, ios{url,sha256}
- (void)checkForUpdate:(void (^)(NSDictionary *_Nullable info, NSString *_Nullable err))done;
// Returns all known versions (newest first). Each dict has the same keys as an update info.
// If the server only has the old single-version format, the array contains just that one.
- (void)fetchVersions:(void (^)(NSArray<NSDictionary *> *_Nullable versions, NSString *_Nullable err))done;
- (void)installUpdate:(NSDictionary *)info;
+ (NSString *)currentVersion;
+ (NSString *)friendly:(NSString *)error;
@end

NS_ASSUME_NONNULL_END
