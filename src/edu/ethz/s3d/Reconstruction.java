package edu.ethz.s3d;

public class Reconstruction {
	protected int x, y, z;
	
	public Reconstruction(int x_in, int y_in, int z_in) {
		x = x_in;
		y = y_in;
		z = z_in;
		generateReconstruction(x, y, z);
	}
	
	private native void generateReconstruction(int x, int y, int z);

	public native void refineWithLatestCut();
}
