//
//  ViewController.swift
//  589tool
//
//  Created by Jian on 2018-02-12.
//  Copyright © 2018 Jian.L. All rights reserved.
//

// simple ios side detecter and data sender.
// will detect red point and send coordinate x y to c++ side.
// hardcoded ip and port !!

import UIKit
import AVKit
import Vision
import CoreFoundation

class ViewController: UIViewController, AVCaptureVideoDataOutputSampleBufferDelegate{
    
    
    
    var inputStream: InputStream!
    var outputStream: OutputStream!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let captureSession = AVCaptureSession()
        
        guard let captureDevice =
            AVCaptureDevice.default(for: .video) else { return }
        guard let input =
            try? AVCaptureDeviceInput(device: captureDevice) else { return }
        captureSession.addInput(input)
        
        let previewLayer = AVCaptureVideoPreviewLayer(session: captureSession)
        view.layer.addSublayer(previewLayer)
        previewLayer.frame = view.frame
        
        let dataOutput = AVCaptureVideoDataOutput()
        dataOutput.videoSettings = [kCVPixelBufferPixelFormatTypeKey as AnyHashable as! String: NSNumber(value: kCVPixelFormatType_32BGRA)]
        dataOutput.setSampleBufferDelegate(self, queue: DispatchQueue(label: "videoQueue"))
        captureSession.addOutput(dataOutput)
        
        captureSession.startRunning()
        
        var readStream: Unmanaged<CFReadStream>?
        var writeStream: Unmanaged<CFWriteStream>?
        
        CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault,
                                           "10.13.98.107" as CFString,
                                           2225,
                                           &readStream,
                                           &writeStream)
        inputStream = readStream!.takeRetainedValue()
        outputStream = writeStream!.takeRetainedValue()
        inputStream.schedule(in: .current, forMode: .commonModes)
        outputStream.schedule(in: .current, forMode: .commonModes)
        inputStream.open()
        outputStream.open()
    }
    
    
    func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        guard
            // get the CVPixelBuffer out of the CMSampleBuffer
            let pixelBuffer: CVPixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer)
            else { return }
        
        CVPixelBufferLockBaseAddress(pixelBuffer,CVPixelBufferLockFlags(rawValue: 0))
        let ptr = CVPixelBufferGetBaseAddress(pixelBuffer)
        let ui8bufptr = UnsafeBufferPointer(start: ptr?.assumingMemoryBound(to: UInt8.self), count: 1080*1920*4)
        let ui8array = Array(ui8bufptr)
        var x = 0
        var y = 0
        var i = 0
        while (i < 1080*1920){
            if ui8array[i*4]<UInt8(40) && ui8array[i*4+1]<UInt8(40) && ui8array[i*4+2]>UInt8(130) {
                x = i % 1920
                y = i / 1920
                break
            }
            i = i + 13
        }
        if x==0 && y==0 {
            print("Not found", Date())
        }else{
            print("Found at", x, y)
            let x4 = UInt8(x % 256)
            x = x / 256
            let x3 = UInt8(x % 256)
            x = x / 256
            let x2 = UInt8(x % 256)
            let x1 = UInt8(x / 256)
            let y4 = UInt8(y % 256)
            y = y / 256
            let y3 = UInt8(y % 256)
            y = y / 256
            let y2 = UInt8(y % 256)
            let y1 = UInt8(y / 256)
            let xy :UnsafePointer<UInt8> = UnsafePointer<UInt8>([x1,x2,x3,x4,y1,y2,y3,y4])
            outputStream.write(xy, maxLength: 8)
        }
        CVPixelBufferUnlockBaseAddress(pixelBuffer,CVPixelBufferLockFlags(rawValue: 0))
        
    }
    
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
    }
}
