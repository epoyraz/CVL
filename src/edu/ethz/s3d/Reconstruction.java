package edu.ethz.s3d;

/**
 * This class is mainly an interface to the native functions which do work.
 * @author gostefan
 */
public class Reconstruction {
	/**
	 * The number of voxels in x direction
	 */
	protected int x;
	/**
	 * The number of voxels in y direction
	 */
	protected int y;
	/**
	 * The number of voxels in z direction
	 */
	protected int z;
	
	/**
	 * This initializes the reconstruction
	 * @param xVoxels Number of voxels in x direction
	 * @param yVoxels Number of voxels in y direction
	 * @param zVoxels Number of voxels in z direction
	 */
	public Reconstruction(int xVoxels, int yVoxels, int zVoxels) {
		x = xVoxels;
		y = yVoxels;
		z = zVoxels;
		generateReconstruction(x, y, z);
	}
	
	/**
	 * This is an interface for the native reconstruction initialization
	 * @param x Number of voxels in x direction
	 * @param y Number of voxels in y direction
	 * @param z Number of voxels in z direction
	 */
	private native void generateReconstruction(int x, int y, int z);

	/**
	 * This is the interface function which initiates the refining of the reconstruction with the latest silhouette
	 */
	public native void refineWithLatestCut();
}
