#import "N1RootViewController.h"
#import <Foundation/Foundation.h>
#import <UIProgressHUD.h>
#import <untar.h>

@implementation N1RootViewController {
	NSArray *packages;
}

- (void)loadView {
	[super loadView];
	self.title = @"n1ghtshade";
	self.tableView.scrollEnabled = NO;
	packages = [[NSArray arrayWithContentsOfURL: [NSURL URLWithString: @"https://synackuk.github.io/n1ghtshade/packages.plist"]] retain];
	if (!packages) {
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle: @"Failed to get packages" message: @"Are you connected to the internet?" delegate:self cancelButtonTitle: @"Ok" otherButtonTitles:nil, nil];
		[alert show];
	}
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	return [packages count];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	return @"Available package managers:";
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"MainCell"];

	if (cell == nil) {
		cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:@"MainCell"];
	}
	NSDictionary *package = packages[indexPath.row];
	cell.textLabel.text = package[@"title"];
	NSData* image_data = [[NSData alloc] initWithContentsOfURL: [NSURL URLWithString: package[@"image"]]];
	cell.imageView.image = [UIImage imageWithData:image_data];
	cell.detailTextLabel.text = package[@"description"];
	if(access([package[@"install_check"] UTF8String], F_OK) == 0) { // Prevent installing twice
		cell.selectionStyle = UITableViewCellSelectionStyleNone;
		cell.userInteractionEnabled = false;
		cell.detailTextLabel.text = [NSString stringWithFormat: @"%@ is already installed.", package[@"title"]];
	}
	else {
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	}
	
	return cell;
}

- (void)actionSheet:(UIActionSheet *)action_sheet clickedButtonAtIndex:(NSInteger)buttonIndex {

	if(action_sheet.cancelButtonIndex == buttonIndex) {
		return;
	}

	NSDictionary *package = packages[action_sheet.tag];
	self.tableView.userInteractionEnabled = NO;
	UIProgressHUD* hud = [[UIProgressHUD alloc] initWithFrame:CGRectZero];
	[hud setText:[NSString stringWithFormat: @"Installing %@", package[@"title"]]];
	[hud showInView:self.tableView];
	
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
	
		NSURL  *package_url = [NSURL URLWithString: package[@"url"]];
		NSData *package_blob = [NSData dataWithContentsOfURL:package_url];
		if (!package_blob) {
			dispatch_async(dispatch_get_main_queue(), ^{
				[hud hide];
				[hud release];
				UIAlertView *alert = [[UIAlertView alloc] initWithTitle: [NSString stringWithFormat: @"Failed to download %@.", package[@"title"]] message: @"Are you connected to the internet?" delegate:nil cancelButtonTitle: @"Ok" otherButtonTitles:nil, nil];
				[alert show];
			});
			return;
		}
		[package_blob writeToFile:@"/tmp/package.tar" atomically:YES];
			
		untar("/tmp/package.tar", "/");
		
		unlink("/tmp/package.tar");
			
		system("/bin/su -c /usr/bin/uicache mobile");
		
		dispatch_async(dispatch_get_main_queue(), ^{
			[hud done];
			[hud hide];
			[hud release];
		});
	
		reboot(1);
	});
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	NSDictionary* package = packages[indexPath.row];

	UIActionSheet* action_sheet = [[UIActionSheet alloc] initWithTitle:nil delegate:(id<UIActionSheetDelegate>)self cancelButtonTitle:@"Cancel" destructiveButtonTitle:nil otherButtonTitles:[NSString stringWithFormat: @"Install %@", package[@"title"]], nil];
	action_sheet.tag = indexPath.row;
	[action_sheet showInView:self.tableView];
	[self.tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
